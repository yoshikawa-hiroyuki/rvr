import sys, os
import struct
import copy
import numpy as np
from scipy.ndimage import zoom
import volume_render as vr

from vfr import *
from vfr import lut
from pySPH import SPH


#----------------------------------------------------------------------
class RVR(gfxNode.GfxNode):
    _RVR_NUMSLICES_INIT = 256
    _RVR_NUMSLICES_OMIT = 40
    
    def __init__(self, name=node.Node._NONAME, suicide=False):
        gfxNode.GfxNode.__init__(self, name, suicide)

        self.p_render = None

        self.m_data_updated = False
        self.m_nslices = RVR._RVR_NUMSLICES_INIT
        self.m_shading = True
        self.m_data = None
        self.m_dims = [0, 0, 0]

        self.m_lut = lut.Lut()
        self.m_lut2 = lut.Lut()
        self.m_lut2.entry[:,3] = 1.0
        
        self.setSpecular([1, 1, 1, 1])
        self.setTransparency(True)
        return

    def renderSolid(self):
        if not self.p_render: return

        self.updateLut()
        self.updateGeom()
        self.updateData()

        self.p_render.Draw()
        return

    def renderWire(self):
        return

    #--------------------------------------------------
    def Initialize(self):
        if self.p_render:
            del self.p_render
        self.p_render = vr.VolumeRender()
        return
    
    def SetData(self, fdata, nx, ny, nz, dmin, dmax, dlen=1, tgt=0):
        dimSz = nx * ny * nz
        if dimSz < 1:
            return False
        if dlen < 1 or tgt >= dlen:
            return False
        if dmin >= dmax:
            return False

        xdims = vr.GetMax3DTexSize([nx, ny, nz])
        xdSz = xdims[0] * xdims[1] * xdims[2]
        if xdSz < 1:
            return False

        data = np.zeros([dimSz], 'uint8')
        d = dmax - dmin
        if d < 1e-8: d = 1e-8

        a = 255.0 / d
        b = -255.0 * dmin / d
        for k in range(nz):
            for j in range(ny):
                for i in range(nx):
                    idx = nx*ny*k + nx*j + i
                    x = fdata[dlen*idx + tgt]
                    if x < dmin: x = dmin
                    elif x > dmax: x = dmax
                    data[idx] = np.uint8(a * x + b)
                    continue # i

        self.m_data = zoom(data.reshape(nz, ny, nx),
                           [xdims[2]/nz, xdims[1]/ny, xdims[0]/nx])
        self.m_dims = self.m_data.shape

        self.m_data_updated = True
        self.notice()
        return True

    def SetLut(self, aLut):
        self.m_lut = aLut
        self.notice()
        return True

    def SetDelibLut(self, aLut):
        self.m_lut2 = aLut
        self.notice()
        return True

    def SetGeom(self, p0, p1):
        self._bbox[0][:] = p0[:]
        self._bbox[1][:] = p1[:]
        self.notice()
        return True
    
    #--------------------------------------------------
    def updateData(self):
        if not self.p_render: return
        if not self.m_data_updated: return
        if self.m_data is None or self.m_dims[0]*self.m_dims[1]*self.m_dims[2] < 1:
            return

        self.p_render.SetVolume(self.m_data,
                                self.m_dims[0], self.m_dims[1], self.m_dims[2])
        self.m_data_updated = False
        return

    def updateLut(self):
        if not self.p_render: return

        rLut = RVR.ConvLut(self.m_lut)
        self.p_render.SetLUT(rLut)

        rLut = RVR.ConvLut(self.m_lut2)
        self.p_render.SetLUT(rLut, True)

        return

    def updateGeom(self):
        if not self.p_render: return

        self.p_render.SetBbox(self._bbox[0].m_v, self._bbox[1].m_v)
        return

    #--------------------------------------------------
    def loadSPH(self, path, didx=0):
        sph = SPH.SPH()
        if not sph.load(path):
            raise RuntimeError('RVR: loadSPH: can not load: {}'.format(path))

        dmin, dmax = (0.0, 0.0)
        if sph.veclen == 1:
            dmin = sph.data.min()
            dmax = sph.data.max()
            data = sph.data.tolist()
        elif sph.veclen > 1 and didx < sph.veclen:
            xdata = sph.dataIndexed()
            dmin = xdata[:,:,:,didx].min()
            dmax = xdata[:,:,:,didx].max()
            data = xdata[:,:,:,didx]\
                .reshape((sph.dims[0]*sph.dims[1]*sph.dims[2])).tolist()
        else:
            raise RuntimeError('RVR: loadSPH: invalid didx specified: {}'\
                               .format(didx))
        self.SetData(data, sph.dims[0], sph.dims[1], sph.dims[2], dmin, dmax)

        gro = [sph.org[0], sph.org[1], sph.org[2]]
        gro[0] += sph.pitch[0] * (sph.dims[0]-1)
        gro[1] += sph.pitch[1] * (sph.dims[1]-1)
        gro[2] += sph.pitch[2] * (sph.dims[2]-1)
        self.SetGeom(sph.org, gro)
        return

    def loadAvsVol(self, path):
        data = None
        dims = None
        with open(path, 'rb') as f:
            buff = struct.unpack('BBB', f.read(3))
            dims = [int(buff[0]), int(buff[1]), int(buff[2])]
            dimSz = dims[0] * dims[1] * dims[2]
            if dimSz < 0:
                raise RuntimeError('RVR: loadAvsVol: not a AVS Volume')

            data = np.zeros((dims[2], dims[1], dims[0]))

            sliceSz = dims[0] * dims[1]
            fmtStr = '{}B'.format(sliceSz)
            for z in range(dims[2]):
                buff = struct.unpack(fmtStr, f.read(sliceSz))
                for y in range(dims[1]):
                    for x in range(dims[0]):
                        data[z, y, x] = float(buff[dims[0]*y + x])
                        continue # x
                    continue # y
                continue # z
            data = data.reshape((dimSz)).tolist()
        if data is None:
            raise RuntimeError('RVR: loadAvsVol: can not read file')

        self.SetData(data, dims[0], dims[1], dims[2], 0.0, 255.0)

        org = [0.0, 0.0, 0.0]
        gro = [dims[0]-1.0, dims[1]-1.0, dims[2]-1.0]
        self.SetGeom(org, gro)
        return
    
    @staticmethod
    def ConvLut(vlut):
        xlut = copy.deepcopy(vlut)
        xlut.normalize()
        uc_lut = np.zeros([lut.LUT_MAX_ENTRY*4], 'uint8')
        for i in range(lut.LUT_MAX_ENTRY):
            idx = i * 4
            uc_lut[idx  ] = np.uint8(xlut.entry[i][0] * 255)
            uc_lut[idx+1] = np.uint8(xlut.entry[i][1] * 255)
            uc_lut[idx+2] = np.uint8(xlut.entry[i][2] * 255)
            uc_lut[idx+3] = np.uint8(xlut.entry[i][3] * 255)
            continue # i
        return uc_lut.tolist()
    

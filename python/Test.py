import wx
import sys, os
from vfr import drawAreaWx
from vfr import camera
from vfr import scene
from vfr import light
from vfr import gfxNode
from vfr import gfxGroup
from vfr import primitives
from vfr import defaultActions
from vfr import utilMath
from vfr import lut
import rvr


app = wx.App(False)
frame = wx.Frame(None, -1, 'Test', size=(600,400))
da = drawAreaWx.DrawAreaWx(frame)
frame.Show(True)

cam = camera.Camera3D()
cam._frustum._eye[2] = 15.0
cam._frustum._dist = 15.0
cam._antiAlias = True
da.setCamera(cam)

scn = scene.Scene('SCENE')
cam.setScene(scn)

li0 = scn.getLight(0)
li0._lightType = light.LT_POINT
li0.trans([5.0, 5.0, 10.0])

rvr_node = rvr.RVR()
rvr_node.Initialize()
rvr_node.loadAvsVol('hydrogen.dat')
rvr_node.SetGeom([-5,-5,-5], [5,5,5])
scn.addChild(rvr_node)

geo_node = primitives.Ball(radius=3.0)
scn.addChild(geo_node)

defaultActions.SetDefaultAction(da)

app.MainLoop()



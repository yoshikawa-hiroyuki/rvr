//
// rvrUtils.cpp
//
#include "rvrUtils.h"
#include "utilEndian.h"

#include <sys/stat.h>
#include <cstring>
#include <cmath>

using namespace std;


//----------------------------------------------------------------------------
//  class rvrSimpleLoader
//----------------------------------------------------------------------------

rvrSimpleLoader::rvrSimpleLoader() : m_data(NULL)
{
  m_size[0] = m_size[1] = m_size[2] = 0;
  m_bbox[0][0] = m_bbox[0][1] = m_bbox[0][2] = 0.f;
  m_bbox[1][0] = m_bbox[1][1] = m_bbox[1][2] = 1.f;
}

rvrSimpleLoader::rvrSimpleLoader(const std::string& path) : m_data(NULL)
{
  m_size[0] = m_size[1] = m_size[2] = 0;
  m_bbox[0][0] = m_bbox[0][1] = m_bbox[0][2] = 0.f;
  m_bbox[1][0] = m_bbox[1][1] = m_bbox[1][2] = 1.f;

  if ( path.size() < 4 ) return;
  string suffix = path.substr(path.size() - 3);
  if ( suffix == "dat" || suffix == "avs" ) {
    LoadAvsVol(path);
  }
  else if ( suffix == "vol" || suffix == "fdv" ) {
    LoadFdvVol(path);
  }
  else if ( suffix == "sph" ) {
    LoadSphVol(path);
  }
}

rvrSimpleLoader::rvrSimpleLoader(const int sx, const int sy, const int sz)
  : m_data(NULL)
{
  m_size[0] = m_size[1] = m_size[2] = 0;
  m_bbox[0][0] = m_bbox[0][1] = m_bbox[0][2] = 0.f;
  m_bbox[1][0] = m_bbox[1][1] = m_bbox[1][2] = 1.f;

  int dimSz = sz * sy * sz;
  if ( dimSz < 1 ) return;
  m_data = new unsigned char[dimSz];
  if ( ! m_data ) return;
  memset(m_data, 0, dimSz);
  m_size[0] = sx;
  m_size[1] = sy;
  m_size[2] = sz;
}

rvrSimpleLoader::~rvrSimpleLoader()
{
  if ( m_data )
    delete [] m_data;
}


bool rvrSimpleLoader::LoadAvsVol(const std::string& path)
{
  if ( m_data ) {
    delete [] m_data; m_data = NULL;
    m_size[0] = m_size[1] = m_size[2] = 0;
  }

  if ( path.size() < 1 ) return false;
  FILE* fp = fopen(path.c_str(), "rb");
  if ( ! fp ) {return false;}

  unsigned char xdim[3];
  if ( fread(xdim, 1, 3, fp) < 3 ) {fclose(fp); return false;}
  int dimSz = xdim[0] * xdim[1] * xdim[2];
  if ( dimSz < 1 ) {fclose(fp); return false;}

  m_data = new unsigned char[dimSz];
  if ( ! m_data ) {fclose(fp); return false;}

  if ( fread(m_data, 1, dimSz, fp) < dimSz ) {
    fclose(fp);
    delete [] m_data; m_data = NULL;
    return false;
  }

  m_size[0] = xdim[0];
  m_size[1] = xdim[1];
  m_size[2] = xdim[2];

  // adjust bbox
  m_bbox[0][0] = m_bbox[0][1] = m_bbox[0][2] = 0.f;
  m_bbox[1][0] = (m_size[0] -1) / 20.;
  m_bbox[1][1] = (m_size[1] -1) / 20.;
  m_bbox[1][2] = (m_size[2] -1) / 20.;
  m_bbox[0][0] = -m_bbox[1][0];
  m_bbox[0][1] = -m_bbox[1][1];
  m_bbox[0][2] = -m_bbox[1][2];

  fclose(fp);
  return true;
}

bool rvrSimpleLoader::LoadFdvVol(const std::string& path)
{
  if ( m_data ) {
    delete [] m_data; m_data = NULL;
    m_size[0] = m_size[1] = m_size[2] = 0;
  }

  if ( path.size() < 1 ) return false;
  FILE* fp = fopen(path.c_str(), "rb");
  if ( ! fp ) {return false;}

  struct stat st_buf;
  if ( fstat(fileno(fp), &st_buf) < 0 ) {fclose(fp); return false;}

  // endian check
  bool doBx = false;
  CES::EMatchType mtf = CES::MatchEndian(fp, 4);
  if ( mtf == CES::UnKnown ) return false;
  if ( mtf == CES::UnMatch ) doBx = true;

  // read dims
  int xdim[5];
  if ( fread(xdim, sizeof(int), 5, fp) < 5 ) {fclose(fp); return false;}
  if ( doBx ) BSWAPVEC(xdim, 5);
  if ( xdim[1] < 1 || xdim[2] < 1 || xdim[3] < 1 ) {fclose(fp); return false;}
  int dims[] = {xdim[1], xdim[2], xdim[3]};
  int dimSz = xdim[1] * xdim[2] * xdim[3];

  // allocate
  m_data = new unsigned char[dimSz];
  if ( ! m_data ) {fclose(fp); return false;}

  // skip to stp
  size_t stp = 0;
  size_t sz = dimSz;
  size_t sz4 = (sz%4==0 ? sz : sz+4-sz%4); // data size with padding
  size_t stpLen = (4 + 8 + 2)*4 + sz4;
  sz = (st_buf.st_size - 5*4) / stpLen; // #of steps
  if ( stp >= sz ) {fclose(fp); return false;}
  int i = 0;
  while ( i < stp ) {
    if ( fseek(fp, stpLen, SEEK_CUR) < 0 ) {fclose(fp); return false;}
    i++;
  }

  // skip time record
  if ( fread(xdim, sizeof(int), 4, fp) < 4 ) {fclose(fp); return false;}

  // read range (= bbox)
  if ( fread(&sz, sizeof(int), 1, fp) < 1 ) {fclose(fp); return false;}
  if ( doBx ) BSWAP32(sz);
  if ( sz != 24 ) {fclose(fp); return false;}
  if ( fread(m_bbox[0], sizeof(float), 3, fp) < 3 ) {
    fclose(fp); return false;
  }
  if ( doBx ) BSWAPVEC(m_bbox[0], 3);
  if ( fread(m_bbox[1], sizeof(float), 3, fp) < 3 ) {
    fclose(fp); return false;
  }
  if ( doBx ) BSWAPVEC(m_bbox[1], 3);
  if ( fread(&sz, sizeof(int), 1, fp) < 1 ) {fclose(fp); return false;}

  // read datas
  if ( fread(&sz, sizeof(int), 1, fp) < 1 ) {fclose(fp); return false;}
  if ( doBx ) BSWAP32(sz);
  if ( sz != sz4 ) {fclose(fp); return false;}
  if ( fread(m_data, 1, dimSz, fp) < dimSz ) {
    fclose(fp); return false;
  }

  m_size[0] = dims[0];
  m_size[1] = dims[1];
  m_size[2] = dims[2];

  // adjust bbox
  m_bbox[0][0] = m_bbox[0][1] = m_bbox[0][2] = 0.f;
  m_bbox[1][0] = (m_size[0] -1) / 20.;
  m_bbox[1][1] = (m_size[1] -1) / 20.;
  m_bbox[1][2] = (m_size[2] -1) / 20.;
  m_bbox[0][0] = -m_bbox[1][0];
  m_bbox[0][1] = -m_bbox[1][1];
  m_bbox[0][2] = -m_bbox[1][2];

  fclose(fp);
  return true;
}

bool rvrSimpleLoader::LoadSphVol(const std::string& path,
				 const double minval, const double maxval)
{
  const double vrange = maxval - minval;
  if ( vrange <= 0.0 ) return false;

  if ( m_data ) {
    delete [] m_data; m_data = NULL;
    m_size[0] = m_size[1] = m_size[2] = 0;
  }

  if ( path.size() < 1 ) return false;
  FILE* fp = fopen(path.c_str(), "rb");
  if ( ! fp ) return false;

  unsigned char buff[32];
  int* pib = (int*)(&buff[4]);
  long long* plb = (long long*)(&buff[4]);
  float* pfb = (float*)(&buff[4]);
  double* pdb = (double*)(&buff[4]);

  // read headers
  if ( fread(buff, 1, 16, fp) < 16 ) {
    fclose(fp); return false;
  }
  size_t dataLen;
  switch ( pib[0] ) {
  case 1: // scalar
    dataLen = 1; break;
  case 2: // vector
    dataLen = 3; break;
  default:
    fclose(fp); return false;
  }
  
  bool dblPrec = false;
  switch ( pib[1] ) {
  case 1: break; // single precision
  case 2: dblPrec = true; break; // double precision
  default:
    fclose(fp); return false;
  }

  // read dims, org, pitch, time
  if ( dblPrec ) {
    if ( fread(buff, 1, 32, fp) < 32 ) {
      fclose(fp); return false;
    }
    m_size[0] = (size_t)plb[0];
    m_size[1] = (size_t)plb[1];
    m_size[2] = (size_t)plb[2];

    if ( fread(buff, 1, 32, fp) < 32 ) {
      fclose(fp); return false;
    }
    m_bbox[0][0] = (float)pdb[0];
    m_bbox[0][1] = (float)pdb[1];
    m_bbox[0][2] = (float)pdb[2];

    if ( fread(buff, 1, 32, fp) < 32 ) {
      fclose(fp); return false;
    }
    double pitch[3];
    pitch[0] = (float)pdb[0];
    pitch[1] = (float)pdb[1];
    pitch[2] = (float)pdb[2];
    m_bbox[1][0] = m_bbox[0][0] + pitch[0] * (m_size[0] - 1);
    m_bbox[1][1] = m_bbox[0][1] + pitch[1] * (m_size[1] - 1);
    m_bbox[1][2] = m_bbox[0][2] + pitch[2] * (m_size[2] - 1);

    if ( fseek(fp, 24 + 4, SEEK_CUR) != 0 ) {
      fclose(fp); return false;
    }
  } else {
    if ( fread(buff, 1, 20, fp) < 20 ) {
      fclose(fp); return false;
    }
    m_size[0] = (size_t)pib[0];
    m_size[1] = (size_t)pib[1];
    m_size[2] = (size_t)pib[2];

    if ( fread(buff, 1, 20, fp) < 20 ) {
      fclose(fp); return false;
    }
    m_bbox[0][0] = pfb[0];
    m_bbox[0][1] = pfb[1];
    m_bbox[0][2] = pfb[2];

    if ( fread(buff, 1, 20, fp) < 20 ) {
      fclose(fp); return false;
    }
    float pitch[3];
    pitch[0] = pfb[0];
    pitch[1] = pfb[1];
    pitch[2] = pfb[2];
    m_bbox[1][0] = m_bbox[0][0] + pitch[0] * (m_size[0] - 1);
    m_bbox[1][1] = m_bbox[0][1] + pitch[1] * (m_size[1] - 1);
    m_bbox[1][2] = m_bbox[0][2] + pitch[2] * (m_size[2] - 1);

    if ( fseek(fp, 16 + 4, SEEK_CUR) != 0 ) {
      fclose(fp); return false;
    }
  }
  size_t dimSz = m_size[0] * m_size[1] * m_size[2];
  if ( dimSz < 1 ) {
    fclose(fp); return false;
  }

  m_data = new unsigned char[dimSz];
  if ( ! m_data ) {fclose(fp); return false;}

  // read data
  size_t k = m_size[0] * dataLen;
  size_t dimSzJK = m_size[1] * m_size[2];
  size_t i, j, idx = 0;
  if ( dblPrec ) {
    double* pDblData = new double[k];
    for ( i = 0; i < dimSzJK; i++ ) {
      if ( fread(pDblData, sizeof(double), k, fp) != k ) {
        delete [] pDblData;
        fclose(fp); return false;
      }
      if ( dataLen > 1 ) {
	for ( j = 0; j < m_size[0]; j++ ) {
	  double val = pDblData[j*3] * pDblData[j*3] 
	    + pDblData[j*3+1] * pDblData[j*3+1] 
	    + pDblData[j*3+2] * pDblData[j*3+2];
	  val = sqrt(val);
	  if ( val > maxval ) val = maxval;
	  else if ( val < minval ) val = minval;
	  m_data[idx++] = (unsigned char)(255.0 * (val - minval) / vrange);
	} // end of for(j)
      } else {
	for ( j = 0; j < m_size[0]; j++ ) {
	  double val = pDblData[j];
	  if ( val > maxval ) val = maxval;
	  else if ( val < minval ) val = minval;
	  m_data[idx++] = (unsigned char)(255.0 * (val - minval) / vrange);
	}
      }
    } // end of for(i)
    delete [] pDblData;
  }
  else {
    float* pFltData = new float[k];
    for ( i = 0; i < dimSzJK; i++ ) {
      if ( fread(pFltData, sizeof(float), k, fp) != k ) {
        delete [] pFltData;
        fclose(fp); return false;
      }
      if ( dataLen > 1 ) {
	for ( j = 0; j < m_size[0]; j++ ) {
	  float val = pFltData[j*3] * pFltData[j*3] 
	    + pFltData[j*3+1] * pFltData[j*3+1] 
	    + pFltData[j*3+2] * pFltData[j*3+2];
	  val = sqrt(val);
	  if ( val > maxval ) val = maxval;
	  else if ( val < minval ) val = minval;
	  m_data[idx++] = (unsigned char)(255.0 * (val - minval) / vrange);
	} // end of for(j)
      } else {
	for ( j = 0; j < m_size[0]; j++ ) {
	  float val = pFltData[j];
	  if ( val > maxval ) val = maxval;
	  else if ( val < minval ) val = minval;
	  m_data[idx++] = (unsigned char)(255.0 * (val - minval) / vrange);
	}
      }
    } // end of for(i)
    delete [] pFltData;
  }

  // adjust bbox
  m_bbox[0][0] = m_bbox[0][1] = m_bbox[0][2] = 0.f;
  m_bbox[1][0] = (m_size[0] -1) / 20.;
  m_bbox[1][1] = (m_size[1] -1) / 20.;
  m_bbox[1][2] = (m_size[2] -1) / 20.;
  m_bbox[0][0] = -m_bbox[1][0];
  m_bbox[0][1] = -m_bbox[1][1];
  m_bbox[0][2] = -m_bbox[1][2];

  fclose(fp);
  return true;
}

bool rvrSimpleLoader::EnPower2()
{
  int dimSz = m_size[0] * m_size[1] * m_size[2];
  if ( dimSz < 1 ) return false;
  if ( ! m_data ) return false;

  if ( IsPow2(m_size[0]) &&
       IsPow2(m_size[1]) &&
       IsPow2(m_size[2]) ) return true;

  int dims[3];
  dims[0] = WrapPow2(m_size[0]);
  dims[1] = WrapPow2(m_size[1]);
  dims[2] = WrapPow2(m_size[2]);
  int dlen = dims[0] * dims[1] * dims[2];
  if ( dlen < 1 ) return false;

  unsigned char* newData = new unsigned char[dlen];
  if ( ! newData ) return false;
  memset(newData, 0, dlen);

  size_t iy, iz;
  unsigned char* orgData = m_data;
  for ( iz = 0; iz < m_size[2]; iz++ )
    for ( iy = 0; iy < m_size[1]; iy++ )
      memcpy(&newData[iz*dims[0]*dims[1] + iy*dims[0]],
             &orgData[iz*m_size[0]*m_size[1] + iy*m_size[0]], m_size[0]);

  delete [] m_data;
  m_data = newData;

  float bbSize[] = {m_bbox[1][0] - m_bbox[0][0],
		    m_bbox[1][1] - m_bbox[0][1],
		    m_bbox[1][2] - m_bbox[0][2]};
  bbSize[0] *= ((float)dims[0] / m_size[0] - 1.0f);
  bbSize[1] *= ((float)dims[1] / m_size[1] - 1.0f);
  bbSize[2] *= ((float)dims[2] / m_size[2] - 1.0f);
  m_bbox[1][0] = m_bbox[1][0] + bbSize[0];
  m_bbox[1][1] = m_bbox[1][1] + bbSize[1];
  m_bbox[1][2] = m_bbox[1][2] + bbSize[2];

  m_size[0] = dims[0];
  m_size[1] = dims[1];
  m_size[2] = dims[2];

  return true;
}

namespace {
  template <class T> inline
  T linearITP(float u, const T& c0, const T& c1) {
    return (1 - u) * c0 + u * c1;
  }

  template <class T> inline
  T bilinearITP(float u, float v,
                const T& c0, const T& c1, const T& c2, const T& c3) {
    return ((1.f-u) * c0 + u * c1) * (1.f-v) + ((1.f-u) * c2 + u * c3) * v;
  }

  template <class T> inline
  T trilinearITP(float u, float v, float w, const T c[8]) {
    T a = bilinearITP(u, v, c[0], c[1], c[2], c[3]);
    T b = bilinearITP(u, v, c[4], c[5], c[6], c[7]);
    return linearITP(w, a, b);
  }
};

bool rvrSimpleLoader::Resample(const int rx, const int ry, const int rz)
{
  if ( m_size[0] < 2 || m_size[1] < 2 || m_size[2] < 2 ||
       rx < 2 || ry < 2 || rz < 2 ) return false;
  if ( ! m_data ) return false;
  if ( m_size[0] == rx && m_size[1] == ry && m_size[2] == rz )
    return true;

  unsigned char* orgData = m_data;
  unsigned char* data = new unsigned char[rx * ry * rz];
  if ( ! data ) return false;

  int orgDim[3] = {m_size[0], m_size[1], m_size[2]};
  int dim[3] = {rx, ry, rz};
  int orgDimX[3] = {m_size[0] -1, m_size[1] -1, m_size[2] -1};
  int dimX[3] = {rx -1, ry -1, rz -1};

  int i, x, y, z, sx, sy, sz, s0 = orgDim[0]*orgDim[1];
  float fx, fy, fz, u, v, w;
  float sval[8];
  for ( i = 0, z = 0; z < dim[2]; z++ ) {
    fz = (float)orgDimX[2] * z / dimX[2];
    sz = (size_t)fz;
    w = fz - sz;
    for ( y = 0; y < dim[1]; y++ ) {
      fy = (float)orgDimX[1] * y / dimX[1];
      sy = (size_t)fy;
      v = fy - sy;
      for ( x = 0; x < dim[0]; x++ ) {
	fx = (float)orgDimX[0] * x / dimX[0];
	sx = (size_t)fx;
	u = fx - sx;
	if ( sx < orgDimX[0] && sy < orgDimX[1] && sz < orgDimX[2] ) {
	  sval[0] = orgData[s0*sz + orgDim[0]*sy + sx];
	  sval[1] = orgData[s0*sz + orgDim[0]*sy + sx+1];
	  sval[2] = orgData[s0*sz + orgDim[0]*(sy+1) + sx];
	  sval[3] = orgData[s0*sz + orgDim[0]*(sy+1) + sx+1];
	  sval[4] = orgData[s0*(sz+1) + orgDim[0]*sy + sx];
	  sval[5] = orgData[s0*(sz+1) + orgDim[0]*sy + sx+1];
	  sval[6] = orgData[s0*(sz+1) + orgDim[0]*(sy+1) + sx];
	  sval[7] = orgData[s0*(sz+1) + orgDim[0]*(sy+1) + sx+1];
	  data[i] = (unsigned char)trilinearITP(u, v, w, sval);
	}
	else {
	  int ssx, ssy, ssz;
	  ssx = (u>0.5f) ? sx+1 : sx;
	  ssy = (v>0.5f) ? sy+1 : sy;
	  ssz = (w>0.5f) ? sz+1 : sz;
	  data[i] = (unsigned char)orgData[s0*ssz + orgDim[0]*ssy + ssx];
	}
	i++;
      } // end of for(x)
    } // end of for(y)
  } // end of for(z)

  delete [] m_data;
  m_data = data;

  float bbSize[] = {m_bbox[1][0] - m_bbox[0][0],
		    m_bbox[1][1] - m_bbox[0][1],
		    m_bbox[1][2] - m_bbox[0][2]};
  bbSize[0] *= ((float)dim[0] / m_size[0] - 1.0f);
  bbSize[1] *= ((float)dim[1] / m_size[1] - 1.0f);
  bbSize[2] *= ((float)dim[2] / m_size[2] - 1.0f);
  m_bbox[1][0] = m_bbox[1][0] + bbSize[0];
  m_bbox[1][1] = m_bbox[1][1] + bbSize[1];
  m_bbox[1][2] = m_bbox[1][2] + bbSize[2];

  m_size[0] = dim[0];
  m_size[1] = dim[1];
  m_size[2] = dim[2];

  return true;
}

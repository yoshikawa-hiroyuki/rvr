//
// rvrUtils.h
//

#ifndef _RVR_UTILS_H_
#define _RVR_UTILS_H_

#include <cstdio>
#include <string>


//----------------------------------------------------------------------------
//  class rvrSimpleLoader
//----------------------------------------------------------------------------

class rvrSimpleLoader {
public:
  unsigned char* m_data;
  int m_size[3];
  float m_bbox[2][3];

  rvrSimpleLoader();
  rvrSimpleLoader(const std::string& path);
  rvrSimpleLoader(const int rx, const int ry, const int rz);
  virtual ~rvrSimpleLoader();

  bool LoadAvsVol(const std::string& path);
  bool LoadFdvVol(const std::string& path);
  bool LoadSphVol(const std::string& path,
		  const double minval = 0.0, const double maxval =1.0);

  bool EnPower2();
  bool Resample(const int sx, const int sy, const int sz);

  static bool IsPow2(const int x);
  static int WrapPow2(const int x);
};

inline bool rvrSimpleLoader::IsPow2(const int x) {
  return ((x > 0) && !(x & (x - 1)));
}

inline int rvrSimpleLoader::WrapPow2(const int x) {
  int d;
  for ( d = 32768; d > 1; d /= 2 )
    if ( d < x ) break;
  d *= 2;
  return d;
}

#endif // _RVR_UTILS_H_


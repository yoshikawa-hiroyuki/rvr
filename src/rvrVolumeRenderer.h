//
// rvrVolumeRenderer.h
//

#ifndef	_RVR_VOLUME_RENDERER_H_
#define	_RVR_VOLUME_RENDERER_H_

#include <GL/glew.h>
#include <cstring>
#include <string>

class rvrVertexShader;
class rvrFragmentShader;
class rvrProgramObject;


//----------------------------------------------------------------------------
//  struct rvrLUT
//----------------------------------------------------------------------------

struct rvrLUT {
  unsigned char m_rgba[256][4];

  rvrLUT() {
    int i;
    for ( i = 0; i < 256; i++ ) {
      //unsigned char r, g, b, a;
      m_rgba[i][0] = (unsigned char)i;
      m_rgba[i][1] = (unsigned char)i / 2;
      m_rgba[i][2] = (unsigned char)(255 - i);
      m_rgba[i][3] = (unsigned char)i;
    }
  }
  rvrLUT(const rvrLUT& org) {*this = org;}
  virtual ~rvrLUT() {}

  void operator=(const rvrLUT& org) {
    memcpy(m_rgba, org.m_rgba, sizeof(m_rgba));
  }
};


//----------------------------------------------------------------------------
//  class rvrVolumeRenderer
//----------------------------------------------------------------------------

class rvrVolumeRenderer {
//protected:
public:
  GLuint m_texture_data;
  GLuint m_texture_lut;
  GLuint m_texture_lut2;
  int    m_size_x;
  int    m_size_y;
  int    m_size_z;

  float  m_near_clipping_length;
  float  m_far_clipping_length;
  int    m_num_of_layers;

  bool   m_is_init_shader;
  rvrProgramObject* m_po;

  bool   m_is_draw_frame;
  float  m_frame_margin_ratio;

  float  m_p0[3];
  float  m_p1[3];

  GLdouble m_clip_plane[6][4];

  bool   m_shading;
  bool   m_gradmap;


  void InitShader();
  void CalcBoundingBox(float& x_min, float& x_max,
		       float& y_min, float& y_max,
		       float& z_min, float& z_max);
  void DrawFrame();
  void EnableClipPlane();
  void DisableClipPlane();
  void ResetClipPlane();

  static std::string rvr_errstr;


public:
  rvrVolumeRenderer(const float n_clip = 0.1f,
		    const float f_clip = 500.1f,
		    const int layer = 256,
		    const bool is_frame = false,
		    const float margin = 0.01f);
  virtual ~rvrVolumeRenderer();

  int SetVolume(const unsigned char *data,
		const int n_size_x,
		const int n_size_y,
		const int n_size_z);
  int UpdateVolume(const unsigned char *data,
		   const int n_size_x,
		   const int n_size_y,
		   const int n_size_z);

  int SetLUT(const rvrLUT& lut, const bool grad = false);
  int UpdateLUT(const rvrLUT& lut, const bool grad = false);
  
  int SetBbox(const float p0[3], const float p1[3]);

  int SetClipPlane(const double p[6]);
  void GetClipPlane(double p[6]) const;

  int Draw(void);

  static std::string GetLastErr();

  inline void SetNearClippingLength(const float d) {
    m_near_clipping_length = d;
  }

  inline void SetFarClippingLength(const float d) {
    m_far_clipping_length = d;
  }

  inline void SetClippingLength(const float n, const float f) {
    m_near_clipping_length = n;
    m_far_clipping_length = f;
  }
  
  inline void SetNumOfLayers(const int n) {
    m_num_of_layers = n;
  }
  
  inline void EnableDrawFrame() {
    m_is_draw_frame = true;
  }
  
  inline void DisableDrawFrame() {
    m_is_draw_frame = false;
  }
  
  inline void SetFrameMarginRatio(const float r) {
    m_frame_margin_ratio = r;
  }
  
  inline void SetShading(const bool mode) {
    m_shading = mode;
  }
  
  inline void SetGradmap(const bool mode) {
    m_gradmap = mode;
  }
};

#endif // _RVR_VOLUME_RENDERER_H_

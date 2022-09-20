//
// rvrVolumeRenderer.cpp
//
#include <iostream>
#include <cmath>
#include <cstring>

#include "rvrProgramObject.h"
#include "rvrVolumeRenderer.h"


using namespace std;

namespace {
  const string s_VERTEX_SHADER_CODE =
    "uniform mat4 modelview_matrix_inverse;\n"
    "varying vec3 p;\n"
    "varying vec3 t;\n"
    "varying vec3 l;\n"
    "void main() {\n"
    "  gl_Position = gl_ProjectionMatrix * gl_Vertex;\n"
    "  gl_ClipVertex = gl_Vertex;\n"
    "  p = ( gl_ModelViewMatrix * gl_Vertex ).xyz;\n"
    "  t = ( modelview_matrix_inverse * gl_Vertex ).xyz;\n"
    "  l = ( modelview_matrix_inverse * gl_LightSource[0].position ).xyz;\n"
    "}\n";

  const string s_FRAGMENT_SHADER_CODE =
    "uniform sampler3D texture_data;\n"
    "uniform float thickness;\n"
    "varying vec3 t;\n"
    "vec4 DensityToColor( float d );\n"
    "void main() {\n"
    "  float d = texture3D( texture_data, t ).x;\n"
    "  vec4  c = DensityToColor( d );\n"
    "  c.a = c.a * thickness;\n"
    "  gl_FragColor = c;\n"
    "}\n";

  const string s_DENSITY_TO_COLOR_CODE =
    "uniform sampler2D texture_lut;\n"
    "uniform sampler2D texture_lut2;\n"
    "uniform int shading;\n"
    "uniform int gradmap;\n"
    "varying vec3 p;\n"
    "varying vec3 t;\n"
    "varying vec3 l;\n"
    "vec3 ComputeGradient();\n"
    "vec4 DensityToColor( float d ) {\n"
    "  vec4 color = texture2D( texture_lut, vec2(d, 0) );\n"
    "  if ( shading == 0 && gradmap == 0 ) return color;\n"
    "  float a = color.a;\n"
    "  vec3 N = ComputeGradient();\n"
    "  if ( gradmap == 1 ) {\n"
    "    float w = length(N) / 1.73205;\n"
    "    vec4 gcol = texture2D( texture_lut2, vec2(w, 0) );\n"
    "    a = a * gcol.a;\n"
    "    color.a = a;\n"
    "    if ( shading == 0 ) return color;\n"
    "  }\n"
    "  if ( length(N) == 0.0 ) {\n"
    "    return color;\n"
    "  }\n"
    "  N = normalize(N);\n"
    "  //vec3 lightVec = l - t;\n"
    "  //vec3 lightVec = gl_LightSource[0].position.xyz - t;\n"
    "  vec3 lightVec = gl_LightSource[0].position.xyz - p;\n"
    "  float dis = length(lightVec);\n"
    "  lightVec = normalize(lightVec);\n"
    "  vec3 lightVec2 = vec3(-lightVec.x, -lightVec.y, -lightVec.z);\n"
    "  vec3 viewVec = normalize(-p);\n"
    "  vec3 halfVec = normalize(lightVec + viewVec);\n"
    "  vec3 halfVec2 = normalize(lightVec2 + viewVec);\n"
    "  float shine = gl_FrontMaterial.shininess;"
    "  float specular = pow(max(dot(N, halfVec), 0.0), shine);\n"
    "  float specular2 = pow(max(dot(N, halfVec2), 0.0), shine);\n"
    "  float attenuation = 1.0 / (gl_LightSource[0].constantAttenuation\n"
    "    + gl_LightSource[0].linearAttenuation * dis\n"
    "    + gl_LightSource[0].quadraticAttenuation * dis * dis);\n"
    "  float diffuse = dot(lightVec, N) * 0.66;\n"
    "  float diffuse2 = dot(lightVec2, N) * 0.33;\n"
    "  color += color * diffuse + color * diffuse2 \n"
    "    + gl_FrontLightProduct[0].ambient\n"
    "    + gl_FrontLightProduct[0].specular * specular * attenuation \n"
    "    + gl_FrontLightProduct[0].specular * specular2 * attenuation;\n"
    "  color.a = a;\n"
    "  return color;\n"
    "}\n";

  const string s_COMPUTE_GRADIENT_CODE =
    "uniform mat4 modelview_matrix_inverse;\n"
    "uniform sampler3D texture_data;\n"
    "uniform vec3 resolution;\n"
    "uniform float bias;\n"
    "varying vec3 t;\n"
    "vec3 ComputeGradient() {\n"
#if 0
    "  float u = textureOffset(texture_data, t, ivec3(-1, 0, 0)).r\n"
    "          - textureOffset(texture_data, t, ivec3(1, 0, 0)).r;\n"
    "  float v = textureOffset(texture_data, t, ivec3(0, -1, 0)).r\n"
    "          - textureOffset(texture_data, t, ivec3(0, 1, 0)).r;\n"
    "  float w = textureOffset(texture_data, t, ivec3(0, 0, -1)).r\n"
    "          - textureOffset(texture_data, t, ivec3(0, 0, 1)).r;\n"
    "  vec3 N = vec3(u, v, w);\n"
#else
    "  vec3 sample1, sample2;\n"
    "  vec3 ofst = bias * resolution / 2.0;\n"
    "  sample1.x = texture3D(texture_data, t-vec3(ofst[0], 0.0, 0.0)).r;\n"
    "  sample2.x = texture3D(texture_data, t+vec3(ofst[0], 0.0, 0.0)).r;\n"
    "  sample1.y = texture3D(texture_data, t-vec3(0.0, ofst[1], 0.0)).r;\n"
    "  sample2.y = texture3D(texture_data, t+vec3(0.0, ofst[1], 0.0)).r;\n"
    "  sample1.z = texture3D(texture_data, t-vec3(0.0, 0.0, ofst[2])).r;\n"
    "  sample2.z = texture3D(texture_data, t+vec3(0.0, 0.0, ofst[2])).r;\n"
    "  vec3 N  = sample1 - sample2;\n"
#endif
    "  vec4 N4 = modelview_matrix_inverse * vec4(N, 0);\n"
    "  return N4.xyz;\n"
    "}\n";

  const GLdouble s_clip_plane[6][4] = {
    {  1,  0,  0, 0 },
    { -1,  0,  0, 1 },
    {  0,  1,  0, 0 },
    {  0, -1,  0, 1 },
    {  0,  0,  1, 0 },
    {  0,  0, -1, 1 }
  };

  bool s_is_glew_inited = false;
} // end of noname namespace


// static
string rvrVolumeRenderer::rvr_errstr;

// static
string rvrVolumeRenderer::GetLastErr() {
  if ( rvr_errstr.empty() ) return rvr_errstr;
  string rs = rvr_errstr;
  rvr_errstr = "";
  return rs;
}

// constructor
rvrVolumeRenderer::rvrVolumeRenderer(const float n_clip, const float f_clip,
				     const int layer,
				     const bool is_frame, const float margin)
  : m_texture_data(0), m_texture_lut(0), m_texture_lut2(0),
    m_size_x(0), m_size_y(0), m_size_z(0),
    m_near_clipping_length(n_clip), m_far_clipping_length(f_clip),
    m_num_of_layers(layer), m_is_init_shader(false), m_po(0),
    m_is_draw_frame(is_frame), m_frame_margin_ratio(margin),
    m_shading(true), m_gradmap(true)
{
  m_p0[0] = m_p0[1] = m_p0[2] = 0.f;
  m_p1[0] = m_p1[1] = m_p1[2] = 1.f;

  memcpy(m_clip_plane, s_clip_plane, sizeof(m_clip_plane));
}

// destructor
rvrVolumeRenderer::~rvrVolumeRenderer()
{
  if ( glIsTexture(m_texture_data) ) {
    glDeleteTextures(1, &m_texture_data);
  }
  if ( glIsTexture(m_texture_lut) ) {
    glDeleteTextures(1, &m_texture_lut);
  }
  if ( glIsTexture(m_texture_lut2) ) {
    glDeleteTextures(1, &m_texture_lut2);
  }
}

int rvrVolumeRenderer::SetVolume(const unsigned char *data,
				 const int n_size_x,
				 const int n_size_y,
				 const int n_size_z)
{
  if ( ! s_is_glew_inited ) {
    glewExperimental = GL_TRUE;
    GLenum glew_error = glewInit();
    if ( glew_error != GLEW_OK ) {
      rvr_errstr = string("GLEW error: ") + 
        string(reinterpret_cast<const char*>(glewGetErrorString(glew_error))) + string("\n");
      return -1;
    }
    s_is_glew_inited = true;
  }

  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // delete old texture
  if ( glIsTexture(m_texture_data) ) {
    glDeleteTextures(1, &m_texture_data);
  }

  // generate new texture
  glGenTextures(1, &m_texture_data);
  glBindTexture(GL_TEXTURE_3D, m_texture_data);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  // store volume data
  m_size_x = n_size_x;
  m_size_y = n_size_y;
  m_size_z = n_size_z;
  glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE,
	       m_size_x, m_size_y, m_size_z,
	       0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);

  return 0;
}

int rvrVolumeRenderer::UpdateVolume(const unsigned char *data,
				    const int n_size_x,
				    const int n_size_y,
				    const int n_size_z)
{
  // check already initialized
  if ( ! glIsTexture(m_texture_data) ) {
    return -1;
  }
  glBindTexture(GL_TEXTURE_3D, m_texture_data);

  // update volume data
  if ( m_size_x != n_size_x || m_size_y != n_size_y || m_size_z != n_size_z ) {
    m_size_x = n_size_x;
    m_size_y = n_size_y;
    m_size_z = n_size_z;
    glTexImage3D(GL_TEXTURE_3D, 0, GL_LUMINANCE, m_size_x, m_size_y, m_size_z,
		 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
  } else {
    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, m_size_x, m_size_y, m_size_z,
		    GL_LUMINANCE, GL_UNSIGNED_BYTE, data);
  }

  return 0;
}

int rvrVolumeRenderer::SetLUT(const rvrLUT& lut, const bool grad)
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  // delete old texture
  if ( ! grad ) {
    if ( glIsTexture(m_texture_lut) ) {
      glDeleteTextures(1, &m_texture_lut);
    }
  } else {
    if ( glIsTexture(m_texture_lut2) ) {
      glDeleteTextures(1, &m_texture_lut2);
    }
  }

  // generate new texture
  if ( ! grad ) {
    glGenTextures(1, &m_texture_lut);
    glBindTexture(GL_TEXTURE_2D, m_texture_lut);
  } else {
    glGenTextures(1, &m_texture_lut2);
    glBindTexture(GL_TEXTURE_2D, m_texture_lut2);
  }
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  // store texture data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, lut.m_rgba);

  return 0;
}

int rvrVolumeRenderer::UpdateLUT(const rvrLUT& lut, const bool grad)
{
  if ( ! grad ) {
    if ( ! glIsTexture(m_texture_lut) ) {
      return -1;
    }
    glBindTexture(GL_TEXTURE_2D, m_texture_lut);
  }
  else {
    if ( ! glIsTexture(m_texture_lut2) ) {
      return -1;
    }
    glBindTexture(GL_TEXTURE_2D, m_texture_lut2);
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 1, 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, lut.m_rgba);

  return 0;
}


int rvrVolumeRenderer::SetBbox(const float p0[3], const float p1[3])
{
  float dx = p1[0] - p0[0];
  float dy = p1[1] - p0[1];
  float dz = p1[2] - p0[2];
  if ( dx < 1e-6f || dy < 1e-6f || dz < 1e-6f )
    return -1;

  m_p0[0] = p0[0]; m_p0[1] = p0[1]; m_p0[2] = p0[2];
  m_p1[0] = p1[0]; m_p1[1] = p1[1]; m_p1[2] = p1[2];
  return 0;
}

int rvrVolumeRenderer::SetClipPlane(const double p[6])
{
  if ( ! p ) return -1;
  for ( int i = 0; i < 6; i++ ) {
    if ( p[i] < 0.0 ) m_clip_plane[i][3] = 0.0;
    else if ( p[i] > 1.0 ) m_clip_plane[i][3] = 1.0;
    else m_clip_plane[i][3] = p[i];
  }
  m_clip_plane[0][3] *= -1;
  m_clip_plane[2][3] *= -1;
  m_clip_plane[4][3] *= -1;
  return 0;
}

void rvrVolumeRenderer::GetClipPlane(double p[6]) const
{
  if ( ! p ) return;
  for ( int i = 0; i < 6; i++ )
    p[i] = m_clip_plane[i][3];
  if ( p[0] < 0.f ) p[0] *= -1;
  if ( p[2] < 0.f ) p[2] *= -1;
  if ( p[4] < 0.f ) p[4] *= -1;
}

void rvrVolumeRenderer::CalcBoundingBox(float& x_min, float& x_max,
					float& y_min, float& y_max,
					float& z_min, float& z_max)
{
  int i;

  // current modelview matrix (local -> camera)
  float	m[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, m);

  // find x_min and x_max
  x_min = m[ 3 * 4 + 0 ];
  x_max = m[ 3 * 4 + 0 ];
  for ( i = 0; i < 3; i++ ) {
    if ( m[ i * 4 + 0 ] < 0 ) {
      x_min += m[ i * 4 + 0 ];
    } else {
      x_max += m[ i * 4 + 0 ];
    }
  } // end of for(i)

  // find y_min and y_max
  y_min = m[ 3 * 4 + 1 ];
  y_max = m[ 3 * 4 + 1 ];
  for ( i = 0; i < 3; i++ ) {
    if ( m[ i * 4 + 1 ] < 0 ) {
      y_min += m[ i * 4 + 1 ];
    } else {
      y_max += m[ i * 4 + 1 ];
    }
  }

  // find z_min and z_max
  z_min = m[ 3 * 4 + 2 ];
  z_max = m[ 3 * 4 + 2 ];
  for ( i = 0; i < 3; i++ ) {
    if ( m[ i * 4 + 2 ] < 0 ) {
      z_min += m[ i * 4 + 2 ];
    } else {
      z_max += m[ i * 4 + 2 ];
    }
  }

  // near and far must be set before exec
  if ( z_max > -m_near_clipping_length ) {
    z_max = -m_near_clipping_length;
  }
  if ( z_min < -m_far_clipping_length ) {
    z_min = -m_far_clipping_length;
  }

  // -m_far_clipping_length < z_min < z_max < -m_near_clipping_length ( < 0 )
}

void rvrVolumeRenderer::InitShader()
{
  if ( GLEW_ARB_vertex_shader && GLEW_ARB_fragment_shader ) {
    ;
  } else {
    rvr_errstr = string("RVR: VolumeRenderer: GLSL not ready\n");
  }

  m_po = new rvrProgramObject();
	
  rvrVertexShader   *vs;
  rvrFragmentShader *fs;
	
  vs = new rvrVertexShader(s_VERTEX_SHADER_CODE);
  m_po->Attach(vs);
	
  fs = new rvrFragmentShader(s_FRAGMENT_SHADER_CODE);
  m_po->Attach(fs);

  fs = new rvrFragmentShader(s_DENSITY_TO_COLOR_CODE);
  m_po->Attach(fs);
	
  fs = new rvrFragmentShader(s_COMPUTE_GRADIENT_CODE);
  m_po->Attach(fs);
	
  m_po->Link();
  rvr_errstr += m_po->GetLastErr();
}

void rvrVolumeRenderer::DrawFrame()
{
  glPushMatrix();

  const float m = m_frame_margin_ratio;
  glTranslatef(-m, -m, -m);
	
  const float s = 1.0f + m * 2;
  glScalef(s, s, s);
  
  glDisable(GL_LIGHTING);
  glColor3f(1.0f, 1.0f, 1.0f);

  int i, j;

  glBegin(GL_LINES);
  for ( i = 0; i <= 1; i++ ) {
    for ( j = 0; j <= 1; j++ ) {
      glVertex3i( i, j, 0 );
      glVertex3i( i, j, 1 );
      glVertex3i( i, 0, j );
      glVertex3i( i, 1, j );
      glVertex3i( 0, i, j );
      glVertex3i( 1, i, j );
    } // end of for(j)
  } // end of for(i)
  glEnd();
	
  glPopMatrix();
}

void rvrVolumeRenderer::EnableClipPlane()
{
  int i;
  for ( i = 0; i < 6; i++ ) {
    glClipPlane(GL_CLIP_PLANE0 + i, m_clip_plane[ i ]);
    glEnable(GL_CLIP_PLANE0 + i);
  }
}

void rvrVolumeRenderer::DisableClipPlane()
{
  int i;
  for ( i = 0; i < 6; i++ ) {
    glDisable(GL_CLIP_PLANE0 + i);
  }
}

void rvrVolumeRenderer::ResetClipPlane()
{
  memcpy(m_clip_plane, s_clip_plane, sizeof(m_clip_plane));
}

int rvrVolumeRenderer::Draw()
{
  // initialize shader
  if ( ! m_is_init_shader ) {
    InitShader();
    m_is_init_shader = true;
  }

  // transformation
  glPushMatrix();
  glTranslatef(m_p0[0], m_p0[1], m_p0[2]);
  glScalef(m_p1[0]-m_p0[0], m_p1[1]-m_p0[1], m_p1[2]-m_p0[2]);

  // draw frame
  if ( m_is_draw_frame ) {
    DrawFrame();
  }

  // check bounding box of volume in camera coordinate
  float	x_min, x_max, y_min, y_max, z_min, z_max;
  CalcBoundingBox(x_min, x_max, y_min, y_max, z_min, z_max);
	
  // set clipping planes
  EnableClipPlane();

  // OpenGL settings
  float thickness = ( z_max - z_min ) / m_num_of_layers;

  int resol = m_size_x;
  if ( resol < m_size_y ) resol = m_size_y;
  if ( resol < m_size_z ) resol = m_size_z;
  float thickness1 = (float)resol / m_num_of_layers;
  float resolution[3] = {1.f / m_size_x, 1.f / m_size_y, 1.f / m_size_z};

  glDisable(GL_CULL_FACE);
  glDisable(GL_LIGHTING);
	
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_3D, m_texture_data);

  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, m_texture_lut);

  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_2D, m_texture_lut2);

  // calc inverse modelview matrix
  // because glsl's ModelViewMatrixInverse is sometimes incorrect...
  float m[ 16 ];
  float mi[ 16 ];
  float d;
  int i;
  glGetFloatv(GL_MODELVIEW_MATRIX, m);
  d = m[ 0 ] * ( m[ 5 ] * m[ 10 ] - m[ 9 ] * m[  6 ] )
    + m[ 4 ] * ( m[ 9 ] * m[  2 ] - m[ 1 ] * m[ 10 ] )
    + m[ 8 ] * ( m[ 1 ] * m[  6 ] - m[ 5 ] * m[  2 ] );
  mi[ 0 ] =    m[ 5 ] * m[ 10 ] - m[ 9 ] * m[ 6 ];
  mi[ 1 ] = -( m[ 1 ] * m[ 10 ] - m[ 9 ] * m[ 2 ] );
  mi[ 2 ] =    m[ 1 ] * m[  6 ] - m[ 5 ] * m[ 2 ];
  mi[ 3 ] = 0.0f;
  mi[ 4 ] = -( m[ 4 ] * m[ 10 ] - m[ 8 ] * m[ 6 ] );
  mi[ 5 ] =    m[ 0 ] * m[ 10 ] - m[ 8 ] * m[ 2 ];
  mi[ 6 ] = -( m[ 0 ] * m[  6 ] - m[ 4 ] * m[ 2 ] );
  mi[ 7 ] = 0.0f;
  mi[ 8 ] =    m[ 4 ] * m[ 9 ] - m[ 8 ] * m[ 5 ];
  mi[ 9 ] = -( m[ 0 ] * m[ 9 ] - m[ 8 ] * m[ 1 ] );
  mi[ 10 ] =    m[ 0 ] * m[ 5 ] - m[ 4 ] * m[ 1 ];
  mi[ 11 ] = 0.0f;
  for ( i = 0; i < 12; i++ ) { mi[ i ] /= d; }
  mi[ 12 ] = -( mi[ 0 ] * m[ 12 ] + mi[ 4 ] * m[ 13 ] + mi[  8 ] * m[ 14 ] );
  mi[ 13 ] = -( mi[ 1 ] * m[ 12 ] + mi[ 5 ] * m[ 13 ] + mi[  9 ] * m[ 14 ] );
  mi[ 14 ] = -( mi[ 2 ] * m[ 12 ] + mi[ 6 ] * m[ 13 ] + mi[ 10 ] * m[ 14 ] );
  mi[ 15 ] = 1.0f;

  // set variables
  m_po->Enable();
  m_po->SetUniform1i("texture_data", 0);
  m_po->SetUniform1i("texture_lut",  1);
  m_po->SetUniform1i("texture_lut2", 2);
  m_po->SetUniform1f("thickness", thickness1);
  m_po->SetUniform1f("bias", 1.0f);
  m_po->SetUniform3f("resolution", resolution[0], resolution[1], resolution[2]);
  m_po->SetUniform1i("shading", (m_shading ? 1 : 0));
  m_po->SetUniform1i("gradmap", (m_gradmap ? 1 : 0));
  m_po->SetMatrix4fv("modelview_matrix_inverse", 1, GL_FALSE, mi);
  rvr_errstr += m_po->GetLastErr();

  // draw proxy polygons
  glPushMatrix();
  glBegin(GL_QUADS);
  for ( i = 0; i < m_num_of_layers; i++ ) {
    float z = z_min + thickness * i;
    glVertex3f( x_min, y_min, z );
    glVertex3f( x_max, y_min, z );
    glVertex3f( x_max, y_max, z );
    glVertex3f( x_min, y_max, z );
  } // end of for(i)
  glEnd();
  glPopMatrix();

  m_po->Disable();

  glDisable(GL_BLEND);
	
  // disable clipping planes
  DisableClipPlane();

  // transformation
  glPopMatrix();

  return 0;
}

//
// rvrShaderObject.cpp
//

#if defined(WINDOWS)
#pragma warning(disable:4267)	// warning for conversion from size_t to int
#endif

#include "rvrShaderObject.h"

#include <iostream>
using namespace std;

static string rvr_errstr;

//----------------------------------------------------------------------------
//  class rvrShaderObject
//----------------------------------------------------------------------------

rvrShaderObject::rvrShaderObject(const string& src, const GLenum shader_type)
  : m_handle(0)
{
  // create OpenGL shader object
  m_handle = glCreateShaderObjectARB(shader_type);
  if ( glGetError() != GL_NO_ERROR ) {
    rvr_errstr = string("RVR: ShaderObject: cannot create shader object\n");
    m_handle = 0;
    return;
  }

  // set source code
  m_source = src;
  const char *s = m_source.c_str();
  int l = m_source.length();
  glShaderSourceARB(m_handle, 1, &s, &l);
  if ( glGetError() != GL_NO_ERROR ) {
    rvr_errstr = string("RVR: ShaderObject: cannot set shader source\n");
    m_handle = 0;
    m_source = "";
    return;
  }

  // compile
  if ( Compile() < 0 ) {
    m_handle = 0;
    m_source = "";
    return;
  }
}


rvrShaderObject::~rvrShaderObject()
{
  if ( m_handle )
    glDeleteObjectARB(m_handle);
}


int rvrShaderObject::Compile()
{
  // check whether error occurred
  if ( glGetError() != GL_NO_ERROR ) {
    rvr_errstr = string("RVR: ShaderObject: Compile(): errors occurred before compiling\n");
    return -1;
  }

  // compile
  glCompileShaderARB(m_handle);

  GLint	result;
  glGetObjectParameterivARB(m_handle, GL_OBJECT_COMPILE_STATUS_ARB, &result);
  if ( glGetError() != GL_NO_ERROR || result == GL_FALSE ) {
    rvr_errstr = string("RVR: ShaderObject::Compile(): cannot compile shader\n");
    int length, l;
    glGetObjectParameterivARB(m_handle, GL_OBJECT_INFO_LOG_LENGTH_ARB, &length);
    if ( length > 0 ) {
      GLcharARB *info_log = new GLcharARB[length];
      glGetInfoLogARB(m_handle, length, &l, info_log);
      rvr_errstr += string(info_log) + string("\n");
      delete [] info_log;
    }
    return -1;
  }

  return 0;
}


//----------------------------------------------------------------------------
//  class rvrVertexShader
//----------------------------------------------------------------------------

rvrVertexShader::rvrVertexShader(const string& src)
  : rvrShaderObject(src, GL_VERTEX_SHADER_ARB)
{
}

rvrVertexShader::~rvrVertexShader()
{
}


//----------------------------------------------------------------------------
//  class rvrFragmentShader
//----------------------------------------------------------------------------

rvrFragmentShader::rvrFragmentShader(const string& src)
  : rvrShaderObject(src, GL_FRAGMENT_SHADER_ARB)
{
}

rvrFragmentShader::~rvrFragmentShader()
{
}

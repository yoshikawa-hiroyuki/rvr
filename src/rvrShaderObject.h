//
// rvrShaderObject.h
//

#ifndef	_RVR_SHADER_OBJECT_H_
#define	_RVR_SHADER_OBJECT_H_

#include	<GL/glew.h>
#ifdef __APPLE__
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <OpenGL/gl3.h> // For Core Profile
#endif

#include	<string>


//----------------------------------------------------------------------------
//  class rvrShaderObject
//----------------------------------------------------------------------------
class rvrShaderObject {
protected:
  GLhandleARB m_handle;
  std::string m_source;

  virtual int Compile();

	
public:
  rvrShaderObject(const std::string& src, const GLenum shader_type);
  virtual ~rvrShaderObject();

  inline GLhandleARB GetHandle() const {
    return m_handle;
  }
};


//----------------------------------------------------------------------------
//  class rvrVertexShader
//----------------------------------------------------------------------------
class rvrVertexShader : public rvrShaderObject {
public:
  rvrVertexShader(const std::string& src);
  virtual ~rvrVertexShader();
};


//----------------------------------------------------------------------------
//  class rvrFragmentShader
//----------------------------------------------------------------------------
class rvrFragmentShader : public rvrShaderObject {
public:
  rvrFragmentShader(const std::string& src);
  virtual ~rvrFragmentShader();
};

#endif // _RVR_SHADER_OBJECT_H_

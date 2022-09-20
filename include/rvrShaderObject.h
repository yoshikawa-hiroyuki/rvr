//
// rvrShaderObject.h
//

#ifndef	_RVR_SHADER_OBJECT_H_
#define	_RVR_SHADER_OBJECT_H_

#include	<GL/glew.h>
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

//
// rvrProgramObject.h
//

#ifndef	_RVR_PROGRAM_OBJECT_H_
#define	_RVR_PROGRAM_OBJECT_H_

#include <GL/glew.h>
#include <iostream>

#include "rvrShaderObject.h"


//----------------------------------------------------------------------------
//  class rvrProgramObject
//----------------------------------------------------------------------------

class rvrProgramObject {
protected:
  GLhandleARB m_handle;
  unsigned m_id;

  static unsigned s_id;

public:
  rvrProgramObject();
  virtual ~rvrProgramObject();

  virtual void Attach(const rvrShaderObject *s);
  virtual int  Link();
  virtual void Use();

  inline void Enable() {
    Use();
  }

  inline void Disable() {
    glUseProgramObjectARB(0);
  }

  inline GLint GetUniformLocation(const char *name) {
    GLint ul = glGetUniformLocationARB(m_handle, name);
    return ul;
  }
  inline GLint GetAttribLocation(const char *name) {
    GLint al = glGetAttribLocationARB(m_handle, name);
    return al;
  }

  // uniform variable
	
  // int
  inline void SetUniform1i(const char *name, GLint v0) {
    glUniform1iARB(GetUniformLocation(name), v0);
  }

  inline void SetUniform2i(const char *name, GLint v0, GLint v1) {
    glUniform2iARB(GetUniformLocation(name), v0, v1);
  }

  inline void SetUniform3i(const char *name, GLint v0, GLint v1, GLint v2) {
    glUniform3iARB(GetUniformLocation(name), v0, v1, v2);
  }

  inline void SetUniform4i(const char *name,
			   GLint v0, GLint v1, GLint v2, GLint v3) {
    glUniform4iARB(GetUniformLocation(name), v0, v1, v2, v3);
  }

  // float
  inline void SetUniform1f(const char *name, GLfloat v0) {
    glUniform1fARB(GetUniformLocation(name), v0);
  }

  inline void SetUniform2f(const char *name, GLfloat v0, GLfloat v1) {
    glUniform2fARB(GetUniformLocation(name), v0, v1);
  }

  inline void SetUniform3f(const char *name,
			   GLfloat v0, GLfloat v1, GLfloat v2) {
    glUniform3fARB(GetUniformLocation(name), v0, v1, v2);
  }

  inline void SetUniform4f(const char *name,
			   GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3) {
    glUniform4fARB(GetUniformLocation(name), v0, v1, v2, v3);
  }

  // int array
  inline void SetUniform1iv(const char *name, GLuint count, const GLint *v) {
    glUniform1ivARB(GetUniformLocation(name), count, v);
  }

  inline void SetUniform2iv(const char *name, GLuint count, const GLint *v) {
    glUniform2ivARB(GetUniformLocation(name), count, v);
  }

  inline void SetUniform3iv(const char *name, GLuint count, const GLint *v) {
    glUniform3ivARB(GetUniformLocation(name), count, v);
  }

  inline void SetUniform4iv(const char *name, GLuint count, const GLint *v) {
    glUniform4ivARB(GetUniformLocation(name), count, v);
  }

  // float array
  inline void SetUniform1fv(const char *name, GLuint count, const GLfloat *v) {
    glUniform1fvARB(GetUniformLocation(name), count, v);
  }

  inline void SetUniform2fv(const char *name, GLuint count, const GLfloat *v) {
    glUniform2fvARB(GetUniformLocation(name), count, v);
  }

  inline void SetUniform3fv(const char *name, GLuint count, const GLfloat *v) {
    glUniform3fvARB(GetUniformLocation(name), count, v);
  }

  inline void SetUniform4fv(const char *name, GLuint count, const GLfloat *v) {
    glUniform4fvARB(GetUniformLocation(name), count, v);
  }

  // matrix
  inline void SetMatrix2fv(const char *name, GLuint count, GLboolean transpose,
			   const GLfloat *v) {
    glUniformMatrix2fvARB(GetUniformLocation(name), count, transpose, v);
  }

  inline void SetMatrix3fv(const char *name, GLuint count, GLboolean transpose,
			   const GLfloat *v) {
    glUniformMatrix3fvARB(GetUniformLocation(name), count, transpose, v);
  }

  inline void SetMatrix4fv(const char *name, GLuint count, GLboolean transpose,
			   const GLfloat *v) {
    glUniformMatrix4fvARB(GetUniformLocation(name), count, transpose, v);
  }

  // attribute variable
	
  // float
  inline void SetAttrib1f(GLint al, GLfloat v0) {
    glVertexAttrib1fARB(al, v0);
  }

  inline void SetAttrib2f(GLint al, GLfloat v0, GLfloat v1) {
    glVertexAttrib2fARB(al, v0, v1);
  }

  inline void SetAttrib3f(GLint al, GLfloat v0, GLfloat v1, GLfloat v2) {
    glVertexAttrib3fARB(al, v0, v1, v2);
  }

  inline void SetAttrib4f(GLint al, GLfloat v0,
			  GLfloat v1, GLfloat v2, GLfloat v3) {
    glVertexAttrib4fARB(al, v0, v1, v2, v3);
  }

  // float array
  inline void SetAttrib1fv(GLint al, const GLfloat *v) {
    glVertexAttrib1fvARB(al, v);
  }

  inline void SetAttrib2fv(GLint al, const GLfloat *v) {
    glVertexAttrib2fvARB(al, v);
  }

  inline void SetAttrib3fv(GLint al, const GLfloat *v) {
    glVertexAttrib3fvARB(al, v);
  }

  inline void SetAttrib4fv(GLint al, const GLfloat *v) {
    glVertexAttrib4fvARB(al, v);
  }


  static std::string rvr_errstr;

  static std::string GetLastErr();
};

#endif	// _RVR_PROGRAM_OBJECT_H__

//
// rvrProgramObject.cpp
//

#include "rvrProgramObject.h"

using namespace std;



//----------------------------------------------------------------------------
//  class rvrProgramObject
//----------------------------------------------------------------------------

unsigned rvrProgramObject::s_id = 0;
string rvrProgramObject::rvr_errstr;

rvrProgramObject::rvrProgramObject()
  : m_handle(0)
{
  m_id = s_id ++;
  m_handle = glCreateProgramObjectARB();
  if ( glGetError() != GL_NO_ERROR ) {
    rvr_errstr = string("RVR: ProgramObject: cannot create program object\n");
    m_handle = 0;
  }
}

rvrProgramObject::~rvrProgramObject()
{
  if ( m_handle )
    glDeleteObjectARB(m_handle);
}


void rvrProgramObject::Attach(const rvrShaderObject *s)
{
  glAttachObjectARB(m_handle, s->GetHandle());
  if ( glGetError() != GL_NO_ERROR ) {
    rvr_errstr = string("RVR: ProgramObject: Attach(): cannot attach shader object\n");
  }
}

int rvrProgramObject::Link()
{
  // check whether error occurred
  if ( glGetError() != GL_NO_ERROR ) {
    rvr_errstr = string("RVR: ProgramObject: Link(): errors occurred before linking\n");
    return -1;
  }

  // link
  glLinkProgramARB(m_handle);

  GLint result;
  glGetObjectParameterivARB(m_handle, GL_OBJECT_LINK_STATUS_ARB, &result);
  if ( glGetError() != GL_NO_ERROR || result == GL_FALSE ) {
    rvr_errstr = string("RVR: ProgramObject: Link(): cannot link program object\n");
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

void rvrProgramObject::Use()
{
  // check whether error occurred
  if ( glGetError() != GL_NO_ERROR ) {
    rvr_errstr = string("RVR: ProgramObject: Use(): errors occurred before using\n");
    //return;
  }

  glUseProgramObjectARB(m_handle);
  if ( glGetError() != GL_NO_ERROR ) {
    rvr_errstr += string("RVR: ProgramObject: Use(): cannot use program object\n");
  }
}

// static
string rvrProgramObject::GetLastErr() {
  if ( rvr_errstr.empty() ) return rvr_errstr;
  string rs = rvr_errstr;
  rvr_errstr = "";
  return rs;
}

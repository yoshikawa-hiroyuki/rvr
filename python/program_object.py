# OpenGL Program Object class

import os, sys
from OpenGL.GL import *
from OpenGL.arrays.vbo import VBO


#----------------------------------------------------------------------
class ProgramObject:
    s_id = 0
    
    def __init__(self):
        self.m_handle = 0
        self.m_id = ProgramObject.s_id
        ProgramObject.s_id += 1

        self.m_handle = glCreateProgram()
        if glGetError() != GL_NO_ERROR:
            raise RuntimeError('ProgramObject: cannot create program object')
        return

    def __del__(self):
        if self.m_handle > 0:
            glDeleteProgram(self.m_handle)
        return

    def Attach(self, shader_obj):
        glAttachShader(self.m_handle, shader_obj.getHandle())
        if glGetError() != GL_NO_ERROR:
            raise RuntimeError('ProgramObject: cannot attach shader object')
        return

    def Link(self):
        if glGetError() != GL_NO_ERROR:
            raise RuntimeError('ProgramObject: Link: errors occurred before linking')
        
        glLinkProgram(self.m_handle)
        """
        result = glGetShaderiv(self.m_handle, GL_LINK_STATUS)
        if not result:
            info_log = glGetShaderInfoLog(self.m_handle)
            raise RuntimeError('ProgramObject: Link: cannot link program object')
        """
        return

    def Use(self):
        if glGetError() != GL_NO_ERROR:
            raise RuntimeError('ProgramObject: Use: errors occurred before using')

        glUseProgram(self.m_handle)
        if glGetError() != GL_NO_ERROR:
            raise RuntimeError('ProgramObject: Use: cannot use program object')

        return

    def Enable(self):
        self.Use()
        return

    def Disable(self):
        glUseProgram(0)
        return

    def GetUniformLocation(self, name):
        ul = glGetUniformLocation(self.m_handle, name)
        return ul

    def GetAttribLocation(self, name):
        al = glGetAttribLocation(self.m_handle, name)
        return al

    # uniform variable
    # int
    def SetUniform1i(self, name, v0):
        glUniform1i(self.GetUniformLocation(name), v0)
        return

    def SetUniform2i(self, name, v0, v1):
        glUniform2i(self.GetUniformLocation(name), v0, v1)
        return

    def SetUniform3i(self, name, v0, v1, v2):
        glUniform3i(self.GetUniformLocation(name), v0, v1, v2)
        return

    def SetUniform4i(self, name, v0, v1, v2, v3):
        glUniform4i(self.GetUniformLocation(name), v0, v1, v2, v3)
        return

    # float
    def SetUniform1f(self, name, v0):
        glUniform1f(self.GetUniformLocation(name), v0)
        return

    def SetUniform2f(self, name, v0, v1):
        glUniform2f(self.GetUniformLocation(name), v0, v1)
        return
    
    def SetUniform3f(self, name, v0, v1, v2):
        glUniform3f(self.GetUniformLocation(name), v0, v1, v2)
        return

    def SetUniform4f(self, name, v0, v1, v2, v3):
        glUniform4f(self.GetUniformLocation(name), v0, v1, v2, v3)
        return

    # matrix
    def SetMatrix2fv(self, name, count, transpose, v):
        glUniformMatrix2fv(self.GetUniformLocation(name), count, transpose, v)
        return

    def SetMatrix3fv(self, name, count, transpose, v):
        glUniformMatrix3fv(self.GetUniformLocation(name), count, transpose, v)
        return

    def SetMatrix4fv(self, name, count, transpose, v):
        glUniformMatrix4fv(self.GetUniformLocation(name), count, transpose, v)
        return

    

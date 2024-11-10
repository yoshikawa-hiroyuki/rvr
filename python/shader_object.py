# OpenGL Shader Object class

import os, sys
import pprint
from OpenGL.GL import *
from OpenGL.arrays.vbo import VBO


#----------------------------------------------------------------------
class ShaderObject(object):
    def __init__(self, shader_src, shader_type):
        self.m_handle = 0
        self.m_source = ''

        self.m_handle = glCreateShader(shader_type)
        if glGetError() != GL_NO_ERROR:
            raise RuntimeError('ShaderObject: cannot create shader object')

        glShaderSource(self.m_handle, shader_src)
        glCompileShader(self.m_handle)
        result = glGetShaderiv(self.m_handle, GL_COMPILE_STATUS)
        if not result:
            info_log = glGetShaderInfoLog(self.m_handle)
            self.m_handle = 0
            self.m_source = ''
            raise RuntimeError('ShaderObject: unable to compile shader')

        return

    def __del__(self):
        if self.m_handle > 0:
            glDeleteShader(self.m_handle)
            self.m_handle = 0
        return

    def getHandle(self):
        return self.m_handle

#----------------------------------------------------------------------
class VertexShaderObject(ShaderObject):
    def __init__(self, shader_src):
        super().__init__(shader_src, GL_VERTEX_SHADER)
        return

#----------------------------------------------------------------------
class FragmentShaderObject(ShaderObject):
    def __init__(self, shader_src):
        super().__init__(shader_src, GL_FRAGMENT_SHADER)
        return

#pragma once

#include <glad/gl.h>
#include <string>
#include <logging.hpp>
#include <cassert>

// General wrapper for OpenGL buffer objects
class GlBuffer
{
   public:
    GLenum bufferType = GL_INVALID_ENUM;
    GLenum usage = GL_INVALID_ENUM;

   private:
    GLuint bufferID = GL_INVALID_INDEX, bindingPoint = GL_INVALID_INDEX;
    GLsizeiptr dataSize = 0;
    GLbitfield flags = 0;

   public:
    GlBuffer(GLenum _bufferType, GLenum _usage = GL_STATIC_DRAW);
    // Used for indexed buffers like Uniform or Shader storage buffers
    void setData(GLuint _bindingPoint, GLsizeiptr dataSize, const void* data);
    // Used for buffers like vertex buffers, pixel buffers, etc.
    void setData(GLsizeiptr _dataSize, const void* data);
    // Used for immutable buffers
    void setData(GLsizeiptr _dataSize, const void* data, GLbitfield _flags);
    void setSubData(const void* data, GLintptr offset = 0);
    void bind();
    GLuint getID();
    GLuint getBindingPoint();
    std::string getBufferTypeString();
    ~GlBuffer();
};

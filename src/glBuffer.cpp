#include <glBuffer.hpp>

GlBuffer::GlBuffer(GLenum _bufferType, GLenum _usage) : bufferType(_bufferType), usage(_usage)
{
    glGenBuffers(1, &bufferID);
    assert(this->bufferID != GL_INVALID_INDEX);
    $info("Created {} buffer object with ID: {}", getBufferTypeString(), this->bufferID);
}

void GlBuffer::bind()
{
    assert(this->bufferID != GL_INVALID_INDEX);
    glBindBuffer(this->bufferType, this->bufferID);
}

void GlBuffer::setData(GLuint _bindingPoint, GLsizeiptr _dataSize, const void* data)
{
    this->bind();
    this->bindingPoint = _bindingPoint;
    this->dataSize = _dataSize;
    glBufferData(this->bufferType, this->dataSize, nullptr, this->usage);
    glBindBufferBase(this->bufferType, this->bindingPoint, this->bufferID);
}

void GlBuffer::setData(GLsizeiptr _dataSize, const void* data)
{
    this->bind();
    this->dataSize = dataSize;
    glBufferData(this->bufferType, this->dataSize, data, this->usage);
}

void GlBuffer::setData(GLsizeiptr _dataSize, const void* data, GLbitfield _flags)
{
    this->bind();
    this->dataSize = dataSize;
    this->flags = _flags;
    glBufferStorage(this->bufferType, this->dataSize, data, this->flags);
}

void GlBuffer::setSubData(const void* data, GLintptr offset)
{
    this->bind();
    glBufferSubData(this->bufferType, offset, this->dataSize, data);
}

GLuint GlBuffer::getID()
{
    return this->bufferID;
}

GLuint GlBuffer::getBindingPoint()
{
    return bindingPoint;
}

std::string GlBuffer::getBufferTypeString()
{
    switch (this->bufferType) {
        case GL_UNIFORM_BUFFER:
            return "Uniform buffer";
        case GL_SHADER_STORAGE_BUFFER:
            return "Shader storage buffer";
        case GL_ATOMIC_COUNTER_BUFFER:
            return "Atomic counter buffer";
        case GL_ARRAY_BUFFER:
            return "Array buffer";
        case GL_ELEMENT_ARRAY_BUFFER:
            return "Element array buffer";
        case GL_PIXEL_PACK_BUFFER:
            return "Pixel pack buffer";
        default:
            return "Unknown buffer type";
    }
}

GlBuffer::~GlBuffer()
{
    glDeleteBuffers(1, &this->bufferID);
}
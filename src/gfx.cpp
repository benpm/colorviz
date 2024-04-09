#include <csignal>
#include <shader_program.hpp>
#include <logging.hpp>
#include <util.hpp>
#include "gfx.hpp"

using namespace gfx;

void catchOpenGLerr(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }

        $critical("GL_{} - {}:{}", error, fs::path(file).filename().string(), line);
    }
}

GLuint gfx::buffer(GLenum target, size_t bytes, const void* data, GLenum usage)
{
    GLuint bufferID = GL_INVALID_INDEX;
    glGenBuffers(1, &bufferID) $glChk;
    glBindBuffer(target, bufferID) $glChk;
    glBufferData(target, bytes, data, usage) $glChk;
    glBindBuffer(target, GL_NONE) $glChk;
    return bufferID;
}

bool gfx::vaoBound(GLuint vaoID)
{
    GLint boundID;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundID) $glChk;
    return static_cast<GLuint>(boundID) == vaoID;
}

bool gfx::vaoBound()
{
    GLint boundID;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &boundID) $glChk;
    return boundID > 0;
}

template <> int gfx::param(GLenum param)
{
    GLint value;
    glGetIntegerv(param, &value) $glChk;
    return static_cast<int>(value);
}

template <> float gfx::param(GLenum param)
{
    GLfloat value;
    glGetFloatv(param, &value) $glChk;
    return static_cast<float>(value);
}

template <> bool gfx::param(GLenum param)
{
    GLboolean value;
    glGetBooleanv(param, &value) $glChk;
    return static_cast<bool>(value);
}

template <> std::string gfx::param(GLenum param)
{
    const GLubyte* value = glGetString(param) $glChk;
    return std::string(reinterpret_cast<const char*>(value));
}

Texture::Texture(const TextureConfig& config)
{
    glGenTextures(1, &this->id) $glChk;
    this->reconfigure(config);
}

void Texture::resize(uint32_t width, uint32_t height)
{
    this->config.width = width;
    this->config.height = height;
    this->reconfigure(this->config);
}

void Texture::reconfigure(const TextureConfig& c)
{
    this->config.internalFormat =
        (this->config.internalFormat ? this->config.internalFormat : c.format);
    this->config = c;

    $assert(c.width > 0 && c.height > 0, "Texture dimensions must be greater than 0");

    glBindTexture(c.target, this->id) $glChk;
    switch (c.target) {
        case GL_TEXTURE_2D:
            glTexImage2D(
                c.target, 0, c.internalFormat, c.width, c.height, 0, c.format, c.storageType, c.data
            ) $glChk;
            break;
        case GL_TEXTURE_CUBE_MAP_ARRAY:
            glTexStorage3D(c.target, 1, c.internalFormat, c.width, c.height, 6 * c.layers) $glChk;
            break;
        case GL_TEXTURE_2D_ARRAY:
            glTexStorage3D(c.target, 1, c.internalFormat, c.width, c.height, c.layers) $glChk;
            break;
        default:
            spdlog::error("Unhandled texture target: {}", c.target);
            break;
    }

    // Shadow maps need special comparison to be read appropriately
    if (c.shadow) {
        glTexParameteri(c.target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE) $glChk;
        glTexParameteri(c.target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL) $glChk;
    }

    // Sets mip filtering depending on whether mipmaps were generated
    if (c.mipmap) {
        glTexParameteri(c.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR) $glChk;
    } else {
        glTexParameteri(c.target, GL_TEXTURE_MIN_FILTER, GL_LINEAR) $glChk;
    }
    glTexParameteri(c.target, GL_TEXTURE_MAG_FILTER, c.filter) $glChk;
    glTexParameteri(c.target, GL_TEXTURE_WRAP_S, c.wrap) $glChk;
    glTexParameteri(c.target, GL_TEXTURE_WRAP_T, c.wrap) $glChk;

    const uint32_t dims = (c.target == GL_TEXTURE_2D) ? 2 : 3;
    if (dims == 3) {
        glTexParameteri(c.target, GL_TEXTURE_WRAP_R, c.wrap) $glChk;
    }

    // Correct border color for shadow maps
    if (c.wrap == GL_CLAMP_TO_BORDER && c.shadow) {
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(c.target, GL_TEXTURE_BORDER_COLOR, borderColor) $glChk;
    }

    // Generate mipmaps if data was provided
    if (c.mipmap && (c.data != nullptr || c.dataCube[0] != nullptr)) {
        glGenerateMipmap(c.target) $glChk;
    }

    glBindTexture(c.target, GL_NONE) $glChk;

    // Reset pointers, we will not allowe reuse of data between reconfigure calls
    this->config.data = nullptr;
    for (int i = 0; i < 6; i++) {
        this->config.dataCube[i] = nullptr;
    }
}

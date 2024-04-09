// OpenGL utilities / wrappers
#pragma once

#include <variant>
#include <type_traits>
#include <unordered_map>
#include <glad/gl.h>
#include <logging.hpp>
#include <vecmath.hpp>
#include <util.hpp>
#include <glBuffer.hpp>

void catchOpenGLerr(const char* file, int line);

// clang-format off
#ifdef BUILD_RELEASE
    #define $glChk ;((void)0);
#else
    #define $glChk ;catchOpenGLerr(__FILE__, (__LINE__)-1);
#endif
// clang-format on

// Helper namespace for opengl
namespace gfx
{
    // Information describing a vertex attribute
    struct VertexAttrInfo
    {
        // Number of elements in attribute
        size_t elements = 0u;
        // Offset in bytes from start of vertex data struct
        size_t byteOffset = 0u;
        // Can be GL_FLOAT, GL_INT, etc.
        GLenum type = GL_INVALID_ENUM;
    };

    /**
     * @brief Create buffer with given target, size, data, and usage
     *
     * @param target GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, etc.
     * @param bytes Size of buffer in bytes
     * @param data Pointer to data to be copied to buffer (null for none)
     * @param usage GL_DYNAMIC_DRAW, GL_DYNAMIC_DRAW, etc.
     * @return GLuint ID of created buffer
     */
    GLuint
    buffer(GLenum target, size_t bytes, const void* data = nullptr, GLenum usage = GL_DYNAMIC_DRAW);

    /**
     * @brief Set buffer data
     *
     * @tparam TContainer container type
     * @param target GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, etc.
     * @param bufID ID of created buffer
     * @param data container of elements to be copied to buffer
     * @param usage GL_DYNAMIC_DRAW, GL_DYNAMIC_DRAW, etc.
     * @param isReAlloc indicates if glBufferData() or glBufferSubData() should be used
     */
    template <typename TContainer>
        requires std::ranges::contiguous_range<TContainer> && std::ranges::sized_range<TContainer>
    void setbuf(
        GLenum target,
        GLuint bufID,
        const TContainer& data,
        GLenum usage = GL_DYNAMIC_DRAW,
        bool isReAlloc = true
    )
    {
        const size_t bytes = std::ranges::size(data) * sizeof(typename TContainer::value_type);

        glBindBuffer(target, bufID) $glChk;
        if (isReAlloc) {
            glBufferData(target, bytes, std::ranges::data(data), usage) $glChk;
        } else {
            glBufferSubData(target, 0, bytes, std::ranges::data(data)) $glChk;
        }
    }

    template <typename TElement>
    void setbuf(
        GLenum target,
        GLuint bufID,
        const TElement* data,
        size_t count,
        GLenum usage = GL_DYNAMIC_DRAW,
        bool isReAlloc = true
    )
    {
        glBindBuffer(target, bufID) $glChk;
        if (isReAlloc) {
            glBufferData(target, count * sizeof(TElement), data, usage) $glChk;
        } else {
            glBufferSubData(target, 0, count * sizeof(TElement), data) $glChk;
        }
    }

    // Check if specific VAO is bound
    bool vaoBound(GLuint vaoID);
    // Check if any VAO is bound
    bool vaoBound();
    // Get parameter value
    template <typename T> T param(GLenum param);
    template <> int param<int>(GLenum param);
    template <> float param<float>(GLenum param);
    template <> bool param<bool>(GLenum param);
    template <> std::string param<std::string>(GLenum param);

    struct TextureConfig
    {
        // GL_TEXTURE_2D, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_2D_ARRAY
        GLenum target = GL_TEXTURE_2D;
        // How you should expect the tex data to be expressed in the shader. GL_RGBA, GL_RGB,
        // GL_DEPTH_COMPONENT, GL_RGBA_INTEGER
        GLenum format = GL_RGBA;
        // Should be same as format, or a more specific sized type like GL_RGBA16F
        GLenum internalFormat = GL_NONE;
        // Width of the texture
        uint32_t width;
        // Height of the texture
        uint32_t height;
        // Texture unit
        GLenum texUnit = GL_TEXTURE0;
        // Wrapping behavior: GL_CLAMP_TO_EDGE, GL_REPEAT, GL_MIRRORED_REPEAT
        GLenum wrap = GL_REPEAT;
        // How the data in the tex is actually stored: GL_UNSIGNED_BYTE, GL_FLOAT, GL_UNSIGNED_INT
        GLenum storageType = GL_UNSIGNED_BYTE;
        // Indicates that mipmaps should be generated
        bool mipmap = false;
        // Indicates that this is a shadow map
        bool shadow = false;
        // Texture filtering: GL_NEAREST, GL_LINEAR
        GLenum filter = GL_LINEAR;
        // Number of layers when specifying a 3D texture, 1 if not 3D
        GLsizei layers = 1;
        // Texture data to upload (will be cleared after upload)
        const void* data = nullptr;
        // Texture data to upload for each face of a cubemap (will be cleared after upload)
        const void* dataCube[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    };

    // Returns number of channels for given opengl format
    static constexpr int getChannels(GLenum format)
    {
        switch (format) {
            case GL_RED:
            case GL_RED_INTEGER:
            case GL_DEPTH_COMPONENT:
            case GL_STENCIL_INDEX:
                return 1;
            case GL_RG:
            case GL_RG_INTEGER:
                return 2;
            case GL_RGB:
            case GL_RGB_INTEGER:
                return 3;
            case GL_RGBA:
            case GL_RGBA_INTEGER:
                return 4;
            default:
                $error("unknown format {}", format);
        }
        return -1;
    }

    struct Texture
    {
       public:
        GLuint id = GL_INVALID_INDEX;
        TextureConfig config;

        Texture() = default;
        Texture(const TextureConfig& config);
        void bind() const
        {
            glActiveTexture(this->config.texUnit) $glChk;
            glBindTexture(this->config.target, this->id) $glChk;
        }
        operator GLuint() const { return this->id; }
        void resize(uint32_t width, uint32_t height);
        void reconfigure(const TextureConfig& config);
        // Returns number of channels
        uint32_t channels() const { return getChannels(this->config.format); }
        // Returns size in elements
        uint32_t size() const
        {
            return this->config.width * this->config.height * this->channels();
        }
        // Grabs the pixel data from this texture, oppitionally appending instead of replacing
        // existing data
        template <typename T> void pixelData(std::vector<T>& data, bool append = false)
        {
            // Using variant to switch on the type of the data
            std::variant<uint8_t, float, uint32_t, int32_t> elementType;
            if constexpr (std::is_base_of_v<Matrix, T>) {
                elementType = { T::Scalar_ };
            } else {
                elementType = T{};
            }

            // Get the number of components per data element
            const uint32_t nComponents = std::visit(
                [](auto arg) -> uint32_t {
                    return static_cast<uint32_t>(sizeof(T) / sizeof(decltype(arg)));
                },
                elementType
            );

            // Determine what the required storage type is
            GLenum reqStorageType = GL_NONE;
            std::visit(
                $overload{
                    [&](uint8_t) { reqStorageType = GL_UNSIGNED_BYTE; },
                    [&](float) { reqStorageType = GL_FLOAT; },
                    [&](uint32_t) { reqStorageType = GL_UNSIGNED_INT; },
                    [&](int32_t) { reqStorageType = GL_INT; },
                },
                elementType
            );

            $assert(
                reqStorageType == this->config.storageType,
                "provided data type {} does not match texture storage type {}", reqStorageType,
                this->config.storageType
            );

            // Write out the buffer contents to the given vector
            size_t offset = 0u;
            if (append) {
                data.resize(data.size() + this->size());
                offset = data.size();
            } else {
                data.resize(this->size());
                offset = 0u;
            }
            glBindTexture(this->config.target, this->id) $glChk;
            glGetTexImage(
                this->config.target, 0, this->config.format, this->config.storageType,
                (void*)&data.data()[offset]
            ) $glChk;
            return std::move(data);
        }
    };

    // Returns the opengl storage type enum for type T
    template <numeric T> static constexpr GLenum getStorageType()
    {
        if constexpr (std::is_same_v<T, bool>) {
            return GL_UNSIGNED_BYTE;
        }
        constexpr bool isSigned = std::is_signed_v<T>;
        constexpr bool isFloat = std::is_floating_point_v<T>;
        if constexpr (sizeof(T) == 1u) {
            return isSigned ? GL_BYTE : GL_UNSIGNED_BYTE;
        } else if constexpr (sizeof(T) == 2u) {
            return isSigned ? GL_SHORT : GL_UNSIGNED_SHORT;
        } else if constexpr (sizeof(T) == 4u) {
            if constexpr (isFloat) {
                return GL_FLOAT;
            }
            return isSigned ? GL_INT : GL_UNSIGNED_INT;
        } else if constexpr (sizeof(T) == 8u && isFloat) {
            return GL_DOUBLE;
        }
        return GL_NONE;
    }

    template <c_Vector TVec>
    TVec getFBPixel(GLuint fbo, GLuint pbo, GLenum attachment, GLenum format, Vector2i pos)
    {
        using TVal = typename TVec::Scalar;
        constexpr int len = TVec::RowsAtCompileTime;
        return getFBPixels<typename TVec::Scalar, len>(fbo, pbo, attachment, format, pos);
    }

    template <numeric T>
    T getFBPixel(GLuint fbo, GLuint pbo, GLenum attachment, GLenum format, Vector2i pos)
    {
        return getFBPixel<Vector<T, 1>>(fbo, pbo, attachment, format, pos).x();
    }

    template <numeric TVal, int channels, int w = 1, int h = w>
    auto getFBPixels(GLuint fbo, GLuint pbo, GLenum attachment, GLenum format, Vector2i pos)
    {
        using TMat = Matrix<TVal, h * channels, w>;

        glBindFramebuffer(GL_FRAMEBUFFER, fbo) $glChk;
        glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo) $glChk;
        if (attachment != GL_DEPTH_ATTACHMENT && attachment != GL_STENCIL_ATTACHMENT &&
            attachment != GL_DEPTH_STENCIL_ATTACHMENT) {
            glReadBuffer(attachment) $glChk;
        }

        constexpr GLenum storageType = getStorageType<TVal>();
        assert(storageType != GL_NONE);
        glReadPixels(std::max(0, pos.x()), std::max(0, pos.y()), w, h, format, storageType, nullptr)
            $glChk;
        TVal* data = (TVal*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY) $glChk;
        if (!data) {
            $error("Failed to map pixel buffer");
        }
        glUnmapBuffer(GL_PIXEL_PACK_BUFFER) $glChk;
        glBindBuffer(GL_PIXEL_PACK_BUFFER, GL_NONE) $glChk;
        glBindFramebuffer(GL_FRAMEBUFFER, GL_NONE) $glChk;
        TMat mat = Map<TMat>(data);
        return mat;
    }
};

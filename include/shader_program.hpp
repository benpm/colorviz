// Abstraction over OpenGL shader program, shaders, and uniforms
#pragma once

#include <util.hpp>
#include <vecmath.hpp>
#include <gfx.hpp>

class Shader
{
   protected:
    mutable bool compiled = false;
    fs::path srcPath = {};
    std::string src = "";

   public:
    std::string label = "unnamed shader";
    mutable GLuint id = GL_INVALID_INDEX;
    const GLenum type = GL_INVALID_ENUM;

    Shader(GLenum type, const fs::path& sourcePath);
    Shader(GLenum type, const std::string& source);
    Shader(GLenum type);
    Shader(const Shader& other);
    Shader& operator=(const Shader& other);
    void setSource(const std::string& source);
    void setPath(const fs::path& sourcePath);
    bool compile() const;
    void attach(GLuint programID) const;
    void detach(GLuint programID) const;
    bool isCompiled() const;

    friend class ShaderProgram;
};

class ShaderProgram
{
   private:
    GLuint id = GL_INVALID_INDEX;
    bool valid = false;
    std::vector<Shader> pipeline;

   public:
    std::string label = "unnamed shader program";

    ShaderProgram();
    ShaderProgram(const std::string& label);
    ShaderProgram(
        const std::vector<Shader>& pipeline,
        const std::string& label = "unnamed shader program"
    );
    // Use this shader program
    void use() const;
    // Set uniform if its name is in given set
    void setUniformCond(
        const char* name,
        const std::unordered_set<std::string>& uniforms,
        const auto& value
    ) const
    {
        if (uniforms.contains(name)) {
            this->setUniform(name, value);
        }
    }
    void setUniform(const char* name, bool value) const;
    void setUniform(const char* name, GLuint value) const;
    void setUniform(const char* name, GLint value) const;
    void setUniform(const char* name, GLfloat value) const;
    void setUniform(const char* name, const Vector2u& value) const;
    void setUniform(const char* name, const Vector2i& value) const;
    void setUniform(const char* name, const Vector2f& value) const;
    void setUniform(const char* name, const Vector3f& value) const;
    void setUniform(const char* name, const Vector4f& value) const;
    void setUniform(const char* name, const Matrix4f& value) const;
    // Gets uniform location or throws an error if not found
    GLint uniformLoc(const char* name) const;
    // Gets the location of an attribute by name
    GLuint attribLoc(const char* name) const;
    // Returns true if attribute exists
    bool hasAttrib(const char* name) const;
    // Binds buffer and sets vertex attribute pointer for a buffer ID
    void setVertexAttrib(
        GLuint bufferID,
        const char* name,
        GLint size,
        GLenum type,
        size_t stride = 0u,
        size_t offset = 0u
    ) const;
    // Returns program ID
    GLuint getID() const;
    // Returns whether program is valid
    bool isValid() const;
    // Returns if bound
    bool isBound() const;
    // Returns if initialized
    bool isInitialized() const;
    // Recompile shader pipeline
    void compile();
    // Compiles from given pipeline if not already initialized, recompiles if already initialized
    void compile(const std::vector<Shader>& pipeline);
};
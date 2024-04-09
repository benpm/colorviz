#include <iostream>
#include <fstream>
#include <sstream>
#include <logging.hpp>
#include <shader_program.hpp>

Shader::Shader(GLenum type) : type(type) {}

Shader::Shader(const Shader& other) : type(other.type)
{
    this->compiled = false;
    this->src = other.src;
    this->srcPath = other.srcPath;
    this->label = other.label;
    this->id = other.id;
}

Shader& Shader::operator=(const Shader& other)
{
    $assert(this->type == other.type, "cannot assign shader of different type");
    this->compiled = false;
    this->src = other.src;
    this->srcPath = other.srcPath;
    this->label = other.label;
    this->id = this->id == GL_INVALID_INDEX ? other.id : this->id;
    return *this;
}

Shader::Shader(GLenum type, const fs::path& sourcePath) : Shader(type)
{
    this->setPath(sourcePath);
    this->label = fs::path(sourcePath).filename().string();
}

Shader::Shader(GLenum type, const std::string& source) : Shader(type)
{
    this->setSource(source);
}

void Shader::setSource(const std::string& source)
{
    this->src = source;
    this->srcPath = "";
}

void Shader::setPath(const fs::path& srcPath)
{
    this->srcPath = srcPath;
    this->src = "";
}

bool Shader::compile() const
{
    if (this->id == GL_INVALID_INDEX) {
        this->id = glCreateShader(this->type) $glChk;
        $debug("created shader [{}]'{}'", this->id, this->label);
    }
    this->compiled = false;
    std::string srcString = this->src;

    // Load shader source from file if specified
    if (!this->srcPath.empty()) {
        std::ifstream sourceFile(this->srcPath);
        if (sourceFile) {
            std::stringstream sourceStream;
            sourceStream << sourceFile.rdbuf();
            srcString = sourceStream.str();
        } else {
            $critical("failed to open {}", this->srcPath.string());
        }
    } else {
        $assert(!srcString.empty(), "Shader '{}' source is empty", this->label);
    }

    // Compile shader and check for success
    const char* sourcePtr = srcString.c_str();
    glShaderSource(this->id, 1, &sourcePtr, nullptr) $glChk;
    glCompileShader(this->id) $glChk;
    GLint success;
    glGetShaderiv(this->id, GL_COMPILE_STATUS, &success) $glChk;
    if (!success) {
        std::string infoLog(2048, '\0');
        glGetShaderInfoLog(this->id, 2048, nullptr, infoLog.data()) $glChk;
        $error("failed to compile '{}':\n{}", this->label, infoLog);
        return this->compiled = false;
    }
    $debug("compiled shader '{}'", this->label);
    return this->compiled = true;
}

void Shader::attach(GLuint programID) const
{
    glAttachShader(programID, this->id) $glChk;
}

void Shader::detach(GLuint programID) const
{
    glDetachShader(programID, this->id) $glChk;
}

bool Shader::isCompiled() const
{
    return this->compiled;
}

// --- ShaderProgram ---

ShaderProgram::ShaderProgram()
{
    this->id = glCreateProgram() $glChk;
}

ShaderProgram::ShaderProgram(const std::string& label) : ShaderProgram()
{
    this->label = label;
}

ShaderProgram::ShaderProgram(const std::vector<Shader>& pipeline, const std::string& label)
    : ShaderProgram(label)
{
    this->compile(this->pipeline);
}

void ShaderProgram::use() const
{
    $assert(this->isValid(), "Shader program '{}' is invalid (didn't link)", this->label);
    glUseProgram(this->id) $glChk;
}

void ShaderProgram::setUniform(const char* name, bool value) const
{
    glUniform1ui(this->uniformLoc(name), value) $glChk;
}

void ShaderProgram::setUniform(const char* name, GLuint value) const
{
    glUniform1ui(this->uniformLoc(name), value) $glChk;
}

void ShaderProgram::setUniform(const char* name, GLint value) const
{
    glUniform1i(this->uniformLoc(name), value) $glChk;
}

void ShaderProgram::setUniform(const char* name, GLfloat value) const
{
    glUniform1f(this->uniformLoc(name), value) $glChk;
}

void ShaderProgram::setUniform(const char* name, const Vector2u& value) const
{
    glUniform2ui(this->uniformLoc(name), value.x(), value.y()) $glChk;
}

void ShaderProgram::setUniform(const char* name, const Vector2i& value) const
{
    glUniform2i(this->uniformLoc(name), value.x(), value.y()) $glChk;
}

void ShaderProgram::setUniform(const char* name, const Vector2f& value) const
{
    glUniform2f(this->uniformLoc(name), value.x(), value.y()) $glChk;
}

void ShaderProgram::setUniform(const char* name, const Vector3f& value) const
{
    glUniform3f(this->uniformLoc(name), value.x(), value.y(), value.z()) $glChk;
}

void ShaderProgram::setUniform(const char* name, const Vector4f& value) const
{
    glUniform4f(this->uniformLoc(name), value.x(), value.y(), value.z(), value.w()) $glChk;
}

void ShaderProgram::setUniform(const char* name, const Matrix4f& value) const
{
    glUniformMatrix4fv(this->uniformLoc(name), 1, GL_FALSE, value.data()) $glChk;
}

GLint ShaderProgram::uniformLoc(const char* name) const
{
    const GLint loc = glGetUniformLocation(this->id, name) $glChk;
    return loc;
}

GLuint ShaderProgram::attribLoc(const char* name) const
{
    const GLint loc = glGetAttribLocation(this->id, name) $glChk;
    return (GLuint)loc;
}

bool ShaderProgram::hasAttrib(const char* name) const
{
    return glGetAttribLocation(this->id, name) >= 0;
}

void ShaderProgram::setVertexAttrib(
    GLuint bufferID,
    const char* name,
    GLint size,
    GLenum type,
    size_t stride,
    size_t offset
) const
{
    this->use();

    if (this->hasAttrib(name)) {
        glBindBuffer(GL_ARRAY_BUFFER, bufferID) $glChk;
        glEnableVertexAttribArray(this->attribLoc(name)) $glChk;
        if (type != GL_INT && type != GL_UNSIGNED_INT) {
            glVertexAttribPointer(
                this->attribLoc(name), size, type, GL_FALSE, stride, (void*)offset
            ) $glChk;
        } else {
            glVertexAttribIPointer(this->attribLoc(name), size, type, stride, (void*)offset) $glChk;
        }
    } else {
        $warn("Attribute '{}' not found in shader program '{}'", name, this->label);
    }
}

GLuint ShaderProgram::getID() const
{
    return this->id;
}

bool ShaderProgram::isValid() const
{
    return this->valid;
}

bool ShaderProgram::isBound() const
{
    GLint boundID;
    glGetIntegerv(GL_CURRENT_PROGRAM, &boundID) $glChk;
    return boundID == (GLint)this->id;
}

bool ShaderProgram::isInitialized() const
{
    return this->pipeline.size() > 0;
}

void ShaderProgram::compile()
{
    for (const Shader& shader : this->pipeline) {
        if (!shader.compile()) {
            this->valid = false;
            return;
        }
    }
    for (const Shader& shader : this->pipeline) {
        shader.attach(this->id);
    }

    // Link the program and log error
    glLinkProgram(this->id) $glChk;
    GLint success;
    glGetProgramiv(this->id, GL_LINK_STATUS, &success) $glChk;
    if (!success) {
        std::string infoLog(2048, '\0');
        glGetProgramInfoLog(this->id, 2048, nullptr, infoLog.data()) $glChk;
        $error("failed to link program '{}':\n{}", label, infoLog);
        this->valid = false;
    } else {
        this->valid = true;
        $info("succesfully linked program '{}'", label);
    }

    // Detach shaders
    for (const Shader& shader : this->pipeline) {
        shader.detach(this->id);
    }
}

void ShaderProgram::compile(const std::vector<Shader>& pipeline)
{
    this->pipeline = pipeline;
    this->compile();
}

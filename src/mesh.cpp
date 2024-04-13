#include <mesh.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

Mesh::Mesh(const std::filesystem::path& filepath, ShaderProgram& _program) : program(_program)
{
    
}

void Mesh::generateBuffers()
{
    glGenVertexArrays(1, &this->vao) $glChk;
    glBindVertexArray(this->vao) $glChk;
    glGenBuffers(1, &this->vbo) $glChk;
    gfx::setbuf(GL_ARRAY_BUFFER, this->vbo, this->vertices);
    this->program.setVertexAttrib(this->vbo, "vPos", 3, GL_FLOAT, 0u, 0u);

    glGenBuffers(1, &this->vboColors) $glChk;
    gfx::setbuf(GL_ARRAY_BUFFER, this->vboColors, this->colors);
    this->program.setVertexAttrib(this->vboColors, "vColor", 3, GL_FLOAT, 0u, 0u);

    glGenBuffers(1, &this->ebo) $glChk;
    gfx::setbuf(GL_ELEMENT_ARRAY_BUFFER, this->ebo, this->triangles);
}

void Mesh::draw()
{
    glBindVertexArray(vao) $glChk;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo) $glChk;
    glDrawElements(GL_TRIANGLES, this->triangles.size() * 3, GL_UNSIGNED_INT, nullptr) $glChk;
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &this->vbo) $glChk;
    glDeleteBuffers(1, &this->ebo) $glChk;
    glDeleteBuffers(1, &this->vboColors) $glChk;
    glDeleteVertexArrays(1, &this->vao) $glChk;
}
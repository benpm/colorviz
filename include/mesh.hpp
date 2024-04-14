#pragma once
#include <util.hpp>
#include <vecmath.hpp>
#include <shader_program.hpp>

class Mesh
{
   public:
    std::string label;
    ShaderProgram& program;
    Vector3f bbMin, bbMax;
    std::vector<Vector3f> vertices;
    std::vector<Vector3f> colors;
    std::vector<Vector3u> triangles;
    Transform3f transform = Transform3f::Identity();

   private:
    GLuint vao, vbo, ebo, vboColors;

   public:
    Mesh(ShaderProgram& _program) : program(_program) {}
    Mesh(const std::filesystem::path& filepath, ShaderProgram& _program);
    Mesh(
        const std::vector<Vector3f>& _vertices,
        const std::vector<Vector3u>& _triangles,
        const std::vector<Vector3f>& _colors,
        ShaderProgram& _program
    );
    Mesh(Mesh& other);
    void setVertexColor(const Vector3f& color);
    void generateBuffers();
    void draw();
    ~Mesh();
};

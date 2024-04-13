#pragma once
#include <util.hpp>
#include <vecmath.hpp>
#include <shader_program.hpp>
class Mesh
{

public:
    ShaderProgram& program;
    Vector3f bbMin, bbMax;
    std::vector<Vector3f> vertices;
    std::vector<Vector3f> colors;
    std::vector<Vector3u> triangles;

   private:
    GLuint vao, vbo, ebo, vboColors;

public:
    Mesh(ShaderProgram& _program): program(_program) {}
    void generateBuffers();
    void draw();
    ~Mesh();
};

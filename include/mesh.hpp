#pragma once
#include <util.hpp>
#include <vecmath.hpp>
#include <shader_program.hpp>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/polygon_mesh_processing.h>

using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Point3 = Kernel::Point_3;
using SurfaceMesh = CGAL::Surface_mesh<Point3>;
namespace PMP = CGAL::Polygon_mesh_processing;

class Mesh
{
   public:
    std::string label;
    bool isActive = true;
    ShaderProgram& program;
    Vector3f bbMin, bbMax;
    std::vector<Vector3f> vertices;
    std::vector<Vector3f> colors;
    std::vector<Vector3u> triangles;
    Transform3f transform = Transform3f::Identity();
    SurfaceMesh surfaceMesh;

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
    void draw(bool isWireframe = false);
    ~Mesh();
};

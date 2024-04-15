#include <mesh.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <execution>

Mesh::Mesh(const std::filesystem::path& modelPath, ShaderProgram& _program) : program(_program)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    std::string fileNameStr = modelPath.string();
    const char* fileName = fileNameStr.c_str();

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, fileName, nullptr, true);
    $assert(ret, "Error loading model from {}:\n{}", modelPath.string(), err);

    for (const auto& shape : shapes) {
        // fill vertices
        for (size_t v = 0; v < attrib.vertices.size(); v += 3u) {
            vertices.emplace_back(Map<Vector3f>(&attrib.vertices[v]));
            this->bbMin = this->bbMin.cwiseMin(vertices.back());
            this->bbMax = this->bbMax.cwiseMax(vertices.back());
            Vector3f color = Map<Vector3f>(&attrib.colors[v]);
            this->colors.push_back(color);
        }
        // fill triangles
        for (size_t i = 0; i < shape.mesh.num_face_vertices.size(); i++) {
            Vector3u triangle = {
                static_cast<unsigned int>(shape.mesh.indices[3 * i + 0].vertex_index),
                static_cast<unsigned int>(shape.mesh.indices[3 * i + 1].vertex_index),
                static_cast<unsigned int>(shape.mesh.indices[3 * i + 2].vertex_index)
            };
            this->triangles.push_back(triangle);
        }
    }
    this->generateBuffers();
    $info(
        "Loaded model from {} with {} vertices and {} triangles", modelPath.string(),
        this->vertices.size(), this->triangles.size()
    );
}

Mesh::Mesh(
    const std::vector<Vector3f>& _vertices,
    const std::vector<Vector3u>& _triangles,
    const std::vector<Vector3f>& _colors,
    ShaderProgram& _program
)
    : Mesh(_program)
{
    this->vertices = _vertices;
    this->triangles = _triangles;
    this->colors = _colors;
    this->generateBuffers();
}

Mesh::Mesh(Mesh& other) : program(other.program)
{
    this->vertices = other.vertices;
    this->triangles = other.triangles;
    this->colors = other.colors;
    this->generateBuffers();
}

void Mesh::setVertexColor(const Vector3f& color)
{
    std::for_each(std::execution::par, this->colors.begin(), this->colors.end(), [&](Vector3f& c) {
        c = color;
    });
    glBindBuffer(GL_ARRAY_BUFFER, this->vboColors) $glChk;
    glBufferSubData(GL_ARRAY_BUFFER, 0, this->colors.size() * sizeof(Vector3f), this->colors.data())
        $glChk;
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

void Mesh::draw(bool isWireframe)
{
    if (!this->isActive) {
        return;
    }
    program.setUniform("uTModel", this->transform.matrix());
    glBindVertexArray(vao) $glChk;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo) $glChk;
    glDrawElements(
        isWireframe ? GL_LINES : GL_TRIANGLES, this->triangles.size() * 3, GL_UNSIGNED_INT, nullptr
    ) $glChk;
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &this->vbo) $glChk;
    glDeleteBuffers(1, &this->ebo) $glChk;
    glDeleteBuffers(1, &this->vboColors) $glChk;
    glDeleteVertexArrays(1, &this->vao) $glChk;
}
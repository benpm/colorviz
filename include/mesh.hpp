// Simple mesh class that can be used to draw 3D UI elements and gizmos
#pragma once

#include <unordered_set>
#include <unordered_map>
#include <util.hpp>
#include <vecmath.hpp>
#include <gfx.hpp>

class Mesh
{
   public:
    GLuint renderType = GL_TRIANGLES;

    struct VertexData
    {
        Vector3f pos;
        Vector3f normal;
    };

   private:
    // Vertex data to be uploaded to vertBuf
    std::vector<VertexData> vertArr;
    // Vertex index data to be uploaded to elemBuf
    std::vector<GLuint> elemArr;

   public:
    // VAO for vertex array
    GLuint vaoID = GL_INVALID_INDEX;
    // VBO for vertices on this patch
    GLuint vertBuf = GL_INVALID_INDEX;
    // EBO for vertex indices on this patch
    GLuint elemBuf = GL_INVALID_INDEX;
    // User-defined ID for this mesh, is written to secondary ID layer
    uint32_t meshID = 0u;

    Mesh();
    Mesh(const std::vector<VertexData>& vertexData, const std::vector<GLuint>& elemData);
    Mesh(const std::vector<VertexData>& vertexData, GLuint _renderType);
    Mesh(fs::path modelPath);

    // Creates a copy of this mesh with new buffers
    Mesh clone() const;

    void draw() const;
    void update();
    void attach();
};

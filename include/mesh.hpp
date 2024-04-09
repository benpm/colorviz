// Simple mesh class that can be used to draw 3D UI elements and gizmos
#pragma once

#include <unordered_set>
#include <unordered_map>
#include <util.hpp>
#include <traits/drawable.hpp>
#include <traits/selectable.hpp>
#include <traits/transformable.hpp>
#include <vecmath.hpp>

class Mesh : public drawable::Data, public transformable::Data
{
   public:
    /* drawable */ struct RenderStates
    {
        const ShaderProgram& prog;
        const Transform3f& parent;
        const uint32_t objectID;
    };
    /* drawable */ struct VertexData
    {
        Vector3f position;
        Vector3f normal;
        Vector3f color = { 0.5f, 0.9f, 0.1f };
    };
    /* drawable */ const static drawable::VertexAttrMap _vertAttrInfo;

    GLuint renderType = GL_TRIANGLES;

   private:
    bool _selected = false;
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
    // Sets all vertex colors to given color
    void setColor(const Vector3f& color);

    /* drawable */ void draw(const RenderStates& s) const;
    /* drawable */ void update();
    /* drawable */ void attach(const RenderStates& s);
    /* drawable */ void setTransform(const Transform3f& t);
    /* drawable */ void setDeltaTransform(const Transform3f& t, bool isRigidTransform);

    /* selectable */ void setSelected(bool s) { this->_selected = s; };
    /* selectable */ bool getSelected() const { return this->_selected; };
};

template <> struct ClassTraits<Mesh>
{
    using traits_t = Traits<Mesh, selectable::Trait, drawable::Trait, transformable::Trait>;
};

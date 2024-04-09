#include <mesh.hpp>
#include <tiny_obj_loader.h>

const drawable::VertexAttrMap Mesh::_vertAttrInfo = {
    { "vPos", { 3u, offsetof(VertexData, position), GL_FLOAT } },
    { "vNormal", { 3u, offsetof(VertexData, normal), GL_FLOAT } },
    { "vColor", { 3u, offsetof(VertexData, color), GL_FLOAT } },
};

Mesh::Mesh()
{
    glGenVertexArrays(1, &vaoID) $glChk;
    glGenBuffers(1, &vertBuf) $glChk;
    glGenBuffers(1, &elemBuf) $glChk;
    $debug(
        "Created mesh {} with VAO {}, vertBuf {}, elemBuf {}", (void*)this, vaoID, vertBuf, elemBuf
    );
}

Mesh::Mesh(const std::vector<VertexData>& vertexData, const std::vector<GLuint>& elemData) : Mesh()
{
    this->vertArr = vertexData;
    this->elemArr = elemData;
}

Mesh::Mesh(const std::vector<VertexData>& vertexData, GLuint _renderType)
    : vertArr(vertexData), renderType(_renderType)
{
    glGenVertexArrays(1, &vaoID) $glChk;
    glGenBuffers(1, &vertBuf) $glChk;
    if (renderType == GL_TRIANGLES) {
        glGenBuffers(1, &elemBuf) $glChk;
    }
    $debug(
        "Created mesh {} with VAO {}, vertBuf {}, elemBuf {}", (void*)this, vaoID, vertBuf, elemBuf
    );
}

Mesh::Mesh(fs::path modelPath) : Mesh()
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    std::string fileNameStr = modelPath.string();
    const char* fileName = fileNameStr.c_str();
    this->label = modelPath.filename().string();

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName, nullptr, true);
    $assert(ret, "Error loading model from {}:\n{}", modelPath.string(), err);

    size_t totalVertices = 0u;
    size_t totalFaces = 0u;
    for (const tinyobj::shape_t& shape : shapes) {
        totalVertices += shape.mesh.indices.size();
        totalFaces += shape.mesh.num_face_vertices.size();
    }
    this->vertArr.reserve(totalVertices);
    this->elemArr.reserve(totalFaces * 3u);

    // Create a patch for each shape, then try to split it
    for (size_t s = 0; s < shapes.size(); s++) {
        // Get degree and vertex indices for each face
        size_t idxOffset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t numVerts = size_t(shapes[s].mesh.num_face_vertices[f]);
            $assert(numVerts == 3u, "Only triangular faces are supported");

            // Loop over vertices in the face.
            for (size_t v = 0; v < numVerts; v++) {
                this->elemArr.push_back(shapes[s].mesh.indices[idxOffset + v].vertex_index);
            }

            idxOffset += numVerts;
        }

        // Create Vector3 vertices from flat array
        for (size_t v = 0; v < attrib.vertices.size() / 3; v++) {
            this->vertArr.emplace_back(VertexData{ Map<Vector3f>(&attrib.vertices[3 * v]) });
        }
    }
}

Mesh Mesh::clone() const
{
    Mesh m;
    m.vertArr = this->vertArr;
    m.elemArr = this->elemArr;
    m.transform = this->transform;
    m.dirty = true;
    return m;
}

void Mesh::setColor(const Vector3f& color)
{
    for (VertexData& v : this->vertArr) {
        v.color = color;
    }
    this->dirty = true;
}

void Mesh::draw(const RenderStates& s) const
{
    if (!this->isRender) {
        return;
    }
    s.prog.use();
    glBindVertexArray(this->vaoID) $glChk;
    glBindBuffer(GL_ARRAY_BUFFER, this->vertBuf) $glChk;
    s.prog.setUniform("uTModel", (s.parent * this->transform).matrix());
    s.prog.setUniform("uMeshID", this->meshID);
    if (this->renderType == GL_TRIANGLES) {
        s.prog.setUniform("uObjectID", s.objectID);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elemBuf) $glChk;
        glDrawElements(GL_TRIANGLES, this->elemArr.size(), GL_UNSIGNED_INT, 0) $glChk;
        return;
    }
    s.prog.setUniform("uObjectID", (uint32_t)0);
    // s.prog.setUniform("uMeshID", 0);
    if (this->renderType == GL_LINES || this->renderType == GL_LINE_LOOP) {
        glLineWidth(2.0f) $glChk;
    }
    glDrawArrays(this->renderType, 0, this->vertArr.size()) $glChk;
}

void Mesh::update()
{
    if (this->dirty) {
        // Update vertex normals
        for (VertexData& v : this->vertArr) {
            v.normal = Vector3f::Zero();
        }
        for (size_t i = 0; i < this->elemArr.size(); i += 3) {
            const GLuint* face = &this->elemArr[i];

            // Update vertex normals
            const Vector3f n = normal(
                this->vertArr[face[0]].position, this->vertArr[face[1]].position,
                this->vertArr[face[2]].position
            );
            for (uint32_t i = 0; i < 3; i++) {
                this->vertArr[face[i]].normal += n;
                this->vertArr[face[i]].normal.normalize();
            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, this->vertBuf) $glChk;
        glBufferData(
            GL_ARRAY_BUFFER, this->vertArr.size() * sizeof(VertexData), this->vertArr.data(),
            GL_DYNAMIC_DRAW
        ) $glChk;
        if (this->renderType == GL_TRIANGLES) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->elemBuf) $glChk;
            glBufferData(
                GL_ELEMENT_ARRAY_BUFFER, this->elemArr.size() * sizeof(GLuint),
                this->elemArr.data(), GL_DYNAMIC_DRAW
            ) $glChk;
        }
        this->dirty = false;
    }
}

void Mesh::attach(const RenderStates& s)
{
    $assert(!this->attached, "already attached");
    drawable::setVertAttribs(*this, s.prog, this->vertBuf);
    this->attached = true;
}

void Mesh::setTransform(const Transform3f& t)
{
    this->transform = t;
}

void Mesh::setDeltaTransform(const Transform3f& t, bool isRigidTransform)
{
    this->transform = t * this->transform;
}

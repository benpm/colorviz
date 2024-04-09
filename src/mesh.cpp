#include <mesh.hpp>

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
    
}

Mesh Mesh::clone() const
{
    Mesh m;
    m.vertArr = this->vertArr;
    m.elemArr = this->elemArr;
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

#version 450

in VertexData
{
    vec3 position;
    vec3 normal;
    vec3 color;
}
v_in;

layout(location = 0) out vec4 fColor;
layout(location = 1) out vec4 fNormal;
layout(location = 2) out uvec3 fObjectID;

layout (std140, binding = 0) uniform EngineState
{
    mat4 uTProj;
    mat4 uTView;
	uint uShowVertices;
    uint uShowNormals;
    uint uShowWireframe;
    uint uIsMatcap;
};
uniform mat4 uTModel;
uniform uint uObjectID;
uniform uint uMeshID;
uniform uint uSelected;

void main()
{
    fNormal = vec4(normalize(v_in.normal), 1.0);
    fColor = vec4(mix(v_in.color, vec3(1.0), float(uSelected)), 1.0);
    if (uObjectID > 0) {
        fObjectID = uvec3(uObjectID, 0, uMeshID);
    }
}
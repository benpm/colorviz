#version 450

in vec3 vPos;
in vec3 vNormal;
in vec3 vColor;

out VertexData
{
    vec3 position;
    vec3 normal;
    vec3 color;
}
v_out;

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

void main()
{
    v_out.position = vPos;
    v_out.normal = normalize(vNormal);
    v_out.color = vColor;
    gl_Position = uTProj * uTView * uTModel * vec4(vPos, 1.0);
}

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

uniform mat4 uTModel;
uniform mat4 uTView;
uniform mat4 uTProj;
uniform bool uIsLAB;
uniform float uExtent;

void main()
{
    v_out.position = vPos;
    v_out.normal = normalize(vNormal);
    v_out.color = vColor;
    if(uIsLAB)
        gl_Position = uTProj * uTView * uTModel * vec4(vPos, 1.0);
    else
        gl_Position = uTProj * uTView * uTModel * vec4(uExtent*vColor, 1.0);
}

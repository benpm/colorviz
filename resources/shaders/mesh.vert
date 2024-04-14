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
uniform float spaceInterp;
uniform float uExtent;

void main()
{
    v_out.position = vPos;
    v_out.normal = normalize(vNormal);
    v_out.color = vColor;
    vec3 interpSpace = mix(vPos, uExtent*vColor, spaceInterp);    
    gl_Position = uTProj * uTView * uTModel * vec4(interpSpace, 1.0);
}

#version 450

in VertexData
{
    vec3 position;
    vec3 normal;
    vec3 color;
}
v_in;

out vec4 fColor;

void main()
{
    fColor = vec4(v_in.color,1.0);
}
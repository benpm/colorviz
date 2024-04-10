#version 450

in VertexData
{
    vec3 position;
    vec3 normal;
    vec3 color;
}
v_in;

out vec4 fColor;

vec3 LABtoXYZ(vec3 lab)
{
    float y = (lab.x + 16.0) / 116.0;
    float x = lab.y / 500.0 + y;
    float z = y - lab.z / 200.0;
    float x3 = x * x * x;
    float z3 = z * z * z;
    return vec3(
        x3 > 0.008856 ? x3 : (x - 16.0 / 116.0) / 7.787,
        lab.x > 903.3 * 0.008856 ? x3 : lab.x / 903.3,
        z3 > 0.008856 ? z3 : (z - 16.0 / 116.0) / 7.787
    );
}

vec3 XYZtoRGB(vec3 xyz)
{
    return vec3(
        xyz.x * 3.2406 + xyz.y * -1.5372 + xyz.z * -0.4986,
        xyz.x * -0.9689 + xyz.y * 1.8758 + xyz.z * 0.0415,
        xyz.x * 0.0557 + xyz.y * -0.2040 + xyz.z * 1.0570
    );
}

vec3 LABtoRGB(vec3 lab)
{
    return XYZtoRGB(LABtoXYZ(lab));
}

void main()
{
    fColor = vec4(LABtoRGB(v_in.position), 1.0);
}
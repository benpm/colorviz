#version 450

in VertexData
{
    vec3 position;
    vec3 normal;
    vec3 color;
}
v_in;

out vec4 fColor;

// D65 white (http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html)
const vec3 refWhite = vec3(0.95047, 1.0, 1.08883);

// sRGB D65 (http://www.brucelindbloom.com/index.html?Eqn_RGB_XYZ_Matrix.html)
const mat3 XYZtoRGBMatrix = mat3( 3.2404542, -1.5371385, -0.4985314,
-0.9692660,  1.8760108,  0.0415560,
 0.0556434, -0.2040259, 1.0572252);

vec3 LABtoXYZ(vec3 lab)
{
    float fy = (lab.x + 16.0) / 116.0;
    float fx = lab.y * 0.002 + fy;
    float fz = fy - lab.z * 0.005;
    float fx3 = fx * fx * fx;
    float fz3 = fz * fz * fz;
    float xr = 0;
    float yr = 0;
    float zr = 0;
    float eps = 216.0/24389.0;
    float k = 24389.0/27.0;
    if (fx3 > eps) xr = fx3;
    else xr = (116.0*fx - 16.0) / k;

    if(lab.x > 7.9996248) yr = pow(fy, 3);
    else yr = lab.x / k;

    if (fz3 > eps) zr = fz3;
    else zr = (116.0*fz - 16.0) / k;

    return vec3(refWhite.x*xr,refWhite.y*yr,refWhite.z*zr);
}

vec3 XYZtoRGB(vec3 xyz)
{
    // return vec3(
    //     xyz.x * 3.2406 + xyz.y * -1.5372 + xyz.z * -0.4986,
    //     xyz.x * -0.9689 + xyz.y * 1.8758 + xyz.z * 0.0415,
    //     xyz.x * 0.0557 + xyz.y * -0.2040 + xyz.z * 1.0570
    // );
    return XYZtoRGBMatrix * xyz;
}

vec3 LABtoRGB(vec3 lab)
{
    vec3 color = XYZtoRGB(LABtoXYZ(lab));
    // gamma correction
    // return pow(color, vec3(1.0/2.2));
    return color;
}

void main()
{
    fColor = vec4(v_in.color,1.0); //vec4(LABtoRGB(v_in.position), 1.0);
}
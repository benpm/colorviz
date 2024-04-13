#pragma once

#include <util.hpp>
#include <vecmath.hpp>
#include <unordered_map>
#include <mesh.hpp>

namespace Gamut
{
    struct GamutData
    {
        std::string descriptor;
        std::string originator;
        std::string created;
        std::string color_rep;
        Vector3f gamut_center;
        Vector3f cspace_white;
        Vector3f gamut_white;
        Vector3f cspace_black;
        Vector3f gamut_black;
    };

    enum class Illuminant
    {
        A,
        B,
        C,
        D50,
        D55,
        D65,
        D75,
        E,
        F2,
        F7,
        F11
    };

    // Reference white points for various illuminants
    // (http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html)
    const std::unordered_map<Illuminant, Vector3f> refWhites{
        { Illuminant::A, { 1.09850f, 1.00000f, 0.35585f } },
        { Illuminant::B, { 0.99072f, 1.00000f, 0.85223f } },
        { Illuminant::C, { 0.98074f, 1.00000f, 1.18232f } },
        { Illuminant::D50, { 0.96422f, 1.00000f, 0.82521f } },
        { Illuminant::D55, { 0.95682f, 1.00000f, 0.92149f } },
        { Illuminant::D65, { 0.95047f, 1.00000f, 1.08883f } },
        { Illuminant::D75, { 0.94972f, 1.00000f, 1.22638f } },
        { Illuminant::E, { 1.00000f, 1.00000f, 1.00000f } },
        { Illuminant::F2, { 0.99186f, 1.00000f, 0.67393f } },
        { Illuminant::F7, { 0.95041f, 1.00000f, 1.08747f } },
        { Illuminant::F11, { 1.00962f, 1.00000f, 0.64350f } }
    };

    const Matrix3f Bradford{ { 0.8951000f, 0.2664000f, -0.1614000f },
                             { -0.7502000f, 1.7135000f, 0.0367000f },
                             { 0.0389000f, -0.0685000f, 1.0296000f } };

    const Matrix3f BradfordInv{ { 0.9869929f, -0.1470543f, 0.1599627f },
                                { 0.4323053f, 0.5183603f, 0.0492912f },
                                { -0.0085287f, 0.0400428f, 0.9684867f } };

    const Matrix3f XYZtoSRGBmatrix{ { 3.2404542, -1.5371385, -0.4985314 },
                                    { -0.9692660, 1.8760108, 0.0415560 },
                                    { 0.0556434, -0.2040259, 1.0572252 } };

    Vector3f LABtoRGB(const Vector3f& lab, Illuminant ill = Illuminant::D65);
    Vector3f XYZtoRGB(Vector3f& color);

    class GamutMesh : public Mesh
    {
        public:
            std::shared_ptr<GamutData> data; 

        public:
            GamutMesh(const std::string& filepath, ShaderProgram& program);
                        
    };
};

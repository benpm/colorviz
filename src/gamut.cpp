#include <gamut.hpp>
using namespace Gamut;

Vector3f Gamut::LABtoRGB(const Vector3f& lab, Illuminant ill)
{
    // Convert CIE Lab to XYZ
    float fy = (lab.x() + 16.0f) / 116.0f;
    float fx = fy + lab.y() / 500.0f;
    float fx3 = fx * fx * fx;
    float fz = fy - lab.z() / 200.0f;
    float fz3 = fz * fz * fz;
    float eps = 216.0f / 24389.0f;
    float k = 24389.0f / 27.0f;
    float xr = fx3 > eps ? fx3 : (116.0f * fx - 16.f) / k;
    float yr = lab.x() > k * eps ? pow((lab.x() + 16.0f) / 116.0f, 3.0f) : lab.x() / k;
    float zr = fz3 > eps ? fz3 : (116.0f * fz - 16.0f) / k;
    Vector3f XYZ = { xr, yr, zr };
    XYZ = XYZ.cwiseProduct(refWhites.at(ill));

    // Convert to D65
    if (ill != Illuminant::D65) {
        Matrix3f scalingMatrix = Matrix3f::Identity();
        scalingMatrix.diagonal() = refWhites.at(Illuminant::D65).cwiseQuotient(refWhites.at(ill));
        XYZ = BradfordInv * scalingMatrix * Bradford * XYZ;
    }
    return XYZtoRGB(XYZ);
}

Vector3f Gamut::XYZtoRGB(Vector3f& color)
{
    color = XYZtoSRGBmatrix.transpose() * color;
    // Correct for gamma
    color.x() =
        color.x() > 0.0031308f ? 1.055f * pow(color.x(), 1.0f / 2.4f) - 0.055f : 12.92f * color.x();
    color.y() =
        color.y() > 0.0031308f ? 1.055f * pow(color.y(), 1.0f / 2.4f) - 0.055f : 12.92f * color.y();
    color.z() =
        color.z() > 0.0031308f ? 1.055f * pow(color.z(), 1.0f / 2.4f) - 0.055f : 12.92f * color.z();
    color = color.cwiseMax(0.0f).cwiseMin(1.0f);
    // clamp to [0, 1]
    return color;
}

enum class GamutSection
{
    header,
    vertices,
    triangle_header,
    triangles
};

std::vector<std::string> parseLine(const std::string& line)
{
    std::stringstream ss(line);
    std::string token;
    std::vector<std::string> tokens;
    while (std::getline(ss, token, ' ')) {
        tokens.push_back(token);
    }
    return tokens;
}

Gamut::GamutMesh::GamutMesh(const std::string& filepath, ShaderProgram& _program)
    : Mesh(_program)
{
    std::ifstream file(filepath);
    assert(file.is_open());

    this->data = std::make_shared<GamutData>();
    std::string line;
    GamutSection section = GamutSection::header;
    this->bbMin = Vector3f::Constant(std::numeric_limits<float>::max());
    this->bbMax = Vector3f::Constant(std::numeric_limits<float>::lowest());
    while (std::getline(file, line)) {
        std::vector<std::string> tokens = parseLine(line);
        if (tokens.empty()) {
            continue;
        }
        switch (section) {
            case GamutSection::header:
                if (tokens[0] == "BEGIN_DATA") {
                    section = GamutSection::vertices;
                }
                break;
            case GamutSection::vertices:
                if (tokens.size() == 4) {
                    this->vertices.push_back({
                        std::stof(tokens[1]),
                        std::stof(tokens[2]),
                        std::stof(tokens[3]),
                    });
                    this->colors.push_back(Gamut::LABtoRGB(this->vertices.back()));
                    this->bbMin = this->bbMin.cwiseMin(this->vertices.back());
                    this->bbMax = this->bbMax.cwiseMax(this->vertices.back());
                } else if (tokens[0] == "END_DATA") {
                    section = GamutSection::triangle_header;
                }
                break;
            case GamutSection::triangle_header:
                if (tokens[0] == "BEGIN_DATA") {
                    section = GamutSection::triangles;
                }
                break;
            case GamutSection::triangles:
                if (tokens.size() == 3) {
                    this->triangles.push_back({
                        (uint32_t)std::stoul(tokens[0]),
                        (uint32_t)std::stoul(tokens[1]),
                        (uint32_t)std::stoul(tokens[2]),
                    });
                }
                break;
        }
    }

    $debug(
        "loaded gamut with {} vertices and {} faces", this->vertices.size(), this->triangles.size()
    );

    this->generateBuffers();
}

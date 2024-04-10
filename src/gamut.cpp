#include <gamut.hpp>

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

enum class GamutSection
{
    header,
    vertices,
    triangle_header,
    triangles
};

std::shared_ptr<GamutData> readGamutData(const std::string& filepath)
{
    std::ifstream file(filepath);
    assert(file.is_open());

    std::shared_ptr<GamutData> data = std::make_shared<GamutData>();
    std::string line;
    GamutSection section = GamutSection::header;
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
                    data->vertices.push_back({
                        std::stof(tokens[1]),
                        std::stof(tokens[2]),
                        std::stof(tokens[3]),
                    });
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
                    data->triangles.push_back({
                        (uint32_t)std::stoul(tokens[0]),
                        (uint32_t)std::stoul(tokens[1]),
                        (uint32_t)std::stoul(tokens[2]),
                    });
                }
                break;
        }
    }

    return data;
}
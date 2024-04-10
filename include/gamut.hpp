#pragma once

#include <util.hpp>
#include <vecmath.hpp>

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
    std::vector<Vector3f> vertices;
    std::vector<Vector3u> triangles;
};

std::shared_ptr<GamutData> readGamutData(const std::string& filepath);
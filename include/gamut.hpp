#include <util.hpp>
#include <vecmath.hpp>

// Struct to represent a gamut vertex
struct GamutVertex {
  double L, A, B;
};

// Struct to represent the entire gamut data
struct GamutData {
  std::string descriptor;
  std::string originator;
  std::string created;
  std::string color_rep;
  GamutVertex gamut_center;
  GamutVertex cspace_white;
  GamutVertex gamut_white;
  GamutVertex cspace_black;
  GamutVertex gamut_black;
  std::vector<GamutVertex> cusps;
  std::vector<std::vector<int>> triangles;
};

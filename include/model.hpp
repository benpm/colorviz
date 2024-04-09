#pragma once

#include <string>
#include <vector>
#include <memory>
#include <util.hpp>
#include <hierarchy.hpp>
#include <surface.hpp>

class Mesh;

struct Model
{
    Model() = delete;

    static void load(fs::path modelPath, Hierarchy& tree, History& history);
    // Generates a quad mesh on the XY plane facing +Z
    static Mesh genQuad(const Transform3f& t = Transform3f::Identity());
    static Mesh genCircle();
};

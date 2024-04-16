#pragma once

#include <shader_program.hpp>
#include <gleq.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <camera.hpp>
#include <vecmath.hpp>
#include <gamut.hpp>

class App
{
   public:
    bool doExit = false;
    GLuint vao, vbo, ebo, vboColors;
    ShaderProgram program;
    Camera cam;
    CameraControl camCtrl;
    int transparentGamut = -1;
    float gamutOpacity = 1.0f;

    float spaceInterpolant = 0.0f, targetSpaceInterpolant = 0.0;
    float startTime = -1.0f;
    bool isAnimateSpace = false;

    std::shared_ptr<Mesh> xAxisArrow, yAxisArrow, zAxisArrow, textL, textA, textB, textR, textG,
        textBcaps;
    uint8_t intersectionHash = 0;  // each bit represent which gamut is intersecting
    std::unordered_map<uint8_t, std::shared_ptr<Mesh>> intersectionMeshes;
    struct Mouse
    {
        Vector2f pos = Vector2f::Zero();
        Vector2f lastPos = Vector2f::Zero();
        Vector2f delta = Vector2f::Zero();
        Vector2f dragStart = Vector2f::Zero();
        Vector2f dragDelta = Vector2f::Zero();
        float scroll = 0.0f;
        bool left = false;
        bool right = false;
        bool disabled = false;
    } mouse;

    std::unordered_set<fs::path> importedGamuts;
    std::vector<std::shared_ptr<Gamut::GamutMesh>> gamuts;

    App(Vector2f winSize);
    // Called before event processing
    void prepare();
    void update(float time, float delta);
    void draw(float time, float delta);
    void updateGUI();
    void event(const GLEQevent& event);
    void onMouseButton(int button, bool pressed);
    void loadGamutMesh(const fs::path& filepath);
    void switchSpace();
    void intersectTwoMeshes(
        std::shared_ptr<Mesh> a,
        std::shared_ptr<Mesh> b,
        uint8_t hash
    );
    void generateIntersectionMesh();
    // if no gamuts are set for intersection, or only one gamut is set, return false
    bool isValidIntersectionHash()
    {
        return (intersectionHash && (intersectionHash & (intersectionHash - 1)));
    }
};
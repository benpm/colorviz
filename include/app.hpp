#pragma once

#include <shader_program.hpp>
#include <gleq.h>
#include <GLFW/glfw3.h>

#include <camera.hpp>
#include <vecmath.hpp>
#include <gamut.hpp>

class App
{
   public:
    bool doExit = false;
    GLuint vao, vbo, ebo;
    ShaderProgram program;
    Camera cam;
    CameraControl camCtrl;
    std::shared_ptr<GamutData> data;
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
    } mouse;

    App(Vector2f winSize);
    // Called before event processing
    void prepare();
    void update(float time, float delta);
    void draw(float time, float delta);
    void event(const GLEQevent& event);
    void onMouseButton(int button, bool pressed);
};
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
    std::shared_ptr<GamutData> data;

    App(Vector2f winSize);
    void prepare();
    void update(float time, float delta);
    void draw(float time, float delta);
    void event(const GLEQevent& event);
};
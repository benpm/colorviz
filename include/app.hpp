#include <shader_program.hpp>
#include <camera.hpp>
#include <gfx.hpp>
#include <GLFW/glfw3.h>
#include <vecmath.hpp>
#include <gamut.hpp>
#include <gleq.h>

class App
{
   public:
    bool doExit = false;
    GLuint vao, vbo, ebo;
    ShaderProgram program;
    Camera cam;
    std::shared_ptr<GamutData> data;

    App(GLFWwindow* window);
    void prepare();
    void update(float time, float delta);
    void draw(float time, float delta);
    void event(const GLEQevent& event);
};
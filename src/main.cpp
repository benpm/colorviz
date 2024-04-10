#include <Eigen/Dense>
#include <gfx.hpp>
#define GLEQ_IMPLEMENTATION
#include <gleq.h>
#include <app.hpp>
#include <cmath>
#ifdef PLATFORM_WINDOWS
    #undef near
    #undef far
#endif
#include <logging.hpp>

int main(int argc, char const* argv[])
{
    setupLogging();

    $assert(
        fs::exists("resources"),
        "resources directory does not exist, you sure you're running from the right place?"
    );

    $assert(glfwInit(), "Failed to initialize GLFW");

    glfwSetErrorCallback([](int error, const char* description) {
        $error("GLFW error [{}] {}", error, description);
    });
    gleqInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "colorviz", nullptr, nullptr);
    $assert(window, "Failed to create GLFW window");

    gleqTrackWindow(window);
    glfwMakeContextCurrent(window);

    bool gladLoaded = gladLoadGL(glfwGetProcAddress);
    $assert(gladLoaded, "Failed to initialize GLAD");

    // Begin the application loop!
    App app({ 1280.0f, 720.0f });
    float t = 0.0f;
    while (!glfwWindowShouldClose(window)) {
        app.prepare();

        // Process events using gleq
        GLEQevent event;
        while (gleqNextEvent(&event)) {
            app.event(event);
            gleqFreeEvent(&event);
        }
        glfwPollEvents();

        // Update and draw, then swap buffers
        app.update(glfwGetTime(), glfwGetTime() - t);
        app.draw(glfwGetTime(), glfwGetTime() - t);
        glfwSwapBuffers(window);

        t = glfwGetTime();

        if (app.doExit) {
            glfwSetWindowShouldClose(window, true);
        }
    }
    return 0;
}

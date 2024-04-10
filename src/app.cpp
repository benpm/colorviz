#include <app.hpp>

App::App(Vector2f winSize)
{
    glfwSwapInterval(1);
    glEnable(GL_DEBUG_OUTPUT) $glChk;
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS) $glChk;
    glEnable(GL_DEPTH_TEST) $glChk;
    glEnable(GL_CULL_FACE) $glChk;
    glCullFace(GL_BACK) $glChk;
    glFrontFace(GL_CCW) $glChk;
    glEnable(GL_BLEND) $glChk;

    program.compile({
        Shader(GL_VERTEX_SHADER, fs::path("resources/shaders/mesh.vert")),
        Shader(GL_FRAGMENT_SHADER, fs::path("resources/shaders/mesh.frag")),
    });

    data = readGamutData("resources/profiles/CIERGB.gam");
    $debug(
        "loaded gamut with {} vertices and {} faces", data->vertices.size(), data->triangles.size()
    );

    glGenBuffers(1, &vbo) $glChk;
    glGenBuffers(1, &ebo) $glChk;
    glGenVertexArrays(1, &vao) $glChk;
    glBindVertexArray(vao) $glChk;
    gfx::setbuf(GL_ARRAY_BUFFER, vbo, data->vertices);
    program.setVertexAttrib(vbo, "vPos", 3, GL_FLOAT, 0u, 0u);
    gfx::setbuf(GL_ELEMENT_ARRAY_BUFFER, ebo, data->triangles);

    camCtrl.mode = CameraControl::Mode::orbit;
    camCtrl.orbitDist(150.0f);
    cam.pos = { 0.0f, 0.0f, 2.0f };
    cam.viewSize = winSize;
}

void App::prepare()
{
    mouse.scroll = 0.0f;
}

void App::update(float time, float delta)
{
    mouse.delta = mouse.pos - mouse.lastPos;
    mouse.dragDelta = Vector2f(mouse.dragStart - mouse.pos) * (float)mouse.left * 2.0f;
    mouse.dragDelta.y() *= -1.0f;
    mouse.lastPos = mouse.pos;

    Vector2f screenDragDelta = mouse.delta * (float)mouse.left;
    camCtrl.control(
        screenDragDelta.cwiseQuotient(cam.viewSize), mouse.dragDelta.cwiseQuotient(cam.viewSize),
        { 0.0f, 0.0f }
    );
    camCtrl.universalZoom(mouse.scroll);
    camCtrl.update(cam, cam.viewSize);
    Matrix4f model = Matrix4f::Identity();
    program.setUniform("uTModel", model);
    program.setUniform("uTView", cam.getView());
    program.setUniform("uTProj", cam.getProj());
}

void App::draw(float time, float delta)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) $glChk;
    program.use();
    glBindVertexArray(vao) $glChk;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo) $glChk;
    glDrawElements(GL_TRIANGLES, data->triangles.size(), GL_UNSIGNED_INT, nullptr) $glChk;
}

void App::event(const GLEQevent& event)
{
    switch (event.type) {
        case GLEQ_KEY_PRESSED:
            if (event.keyboard.key == GLFW_KEY_ESCAPE) {
                doExit = true;
            }
            break;
        case GLEQ_WINDOW_RESIZED:
            cam.viewSize.x() = event.size.width;
            cam.viewSize.y() = event.size.height;
            glViewport(0, 0, event.size.width, event.size.height) $glChk;
            break;
        case GLEQ_CURSOR_MOVED:
            mouse.pos = { event.pos.x, event.pos.y };
            break;
        case GLEQ_BUTTON_PRESSED:
            onMouseButton(event.mouse.button, true);
            break;
        case GLEQ_BUTTON_RELEASED:
            onMouseButton(event.mouse.button, false);
            break;
        case GLEQ_SCROLLED:
            mouse.scroll += -event.scroll.y * 0.1f;
            break;
    }
}

void App::onMouseButton(int button, bool pressed)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (mouse.left ^ pressed) {
            // Mouse button just pressed
            camCtrl.dragStart();
            mouse.dragStart = mouse.pos;
        }
        mouse.left = pressed;
    } else if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        mouse.right = pressed;
    }
}

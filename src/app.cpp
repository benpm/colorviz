#include <app.hpp>

App::App(Vector2f winSize)
{
    glfwSwapInterval(1);
    glEnable(GL_DEBUG_OUTPUT) $glChk;
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS) $glChk;
    glEnable(GL_DEPTH_TEST) $glChk;
    glFrontFace(GL_CCW) $glChk;
    glEnable(GL_BLEND) $glChk;

    program.compile({
        Shader(GL_VERTEX_SHADER, fs::path("resources/shaders/mesh.vert")),
        Shader(GL_FRAGMENT_SHADER, fs::path("resources/shaders/mesh.frag")),
    });

    this->gamutMeshes.emplace_back(
        std::make_shared<Gamut::GamutMesh>("resources/profiles/CIERGB.gam", program)
    );
    this->gamutMeshes.emplace_back(
        std::make_shared<Gamut::GamutMesh>("resources/profiles/AdobeRGB1998.gam", program)
    );

    camCtrl.mode = CameraControl::Mode::trackball;
    camCtrl.orbitDist(500.0f);
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
    Transform3f model = Transform3f::Identity();
    model.rotate(AngleAxisf(45, Vector3f::UnitZ()));
    Vector3f centroid =
        gamutMeshes[0]->bbMin + (gamutMeshes[0]->bbMax - gamutMeshes[0]->bbMin) / 2.0f;
    model.translate(-centroid);
    program.setUniform("uTModel", model.matrix());
    program.setUniform("uTView", cam.getView());
    program.setUniform("uTProj", cam.getProj());
}

void App::draw(float time, float delta)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) $glChk;
    program.use();
    for(auto& mesh : gamutMeshes) {
        mesh->draw();
    }
    
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

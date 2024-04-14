#include <app.hpp>
#include <filesystem>
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

    this->loadGamutMesh("resources/profiles/CIERGB.gam");
    this->loadGamutMesh("resources/profiles/AdobeRGB1998.gam");
    this->gamutMeshes[0]->isWireframe = true;

    yAxisArrow =
        std::make_shared<Mesh>(std::filesystem::path("resources/models/arrow.obj"), program);
    yAxisArrow->setVertexColor({ 0.0f, 1.0f, 0.0f });
    yAxisArrow->transform.scale(25.f);

    xAxisArrow = std::make_shared<Mesh>(*yAxisArrow);
    xAxisArrow->setVertexColor({ 1.0f, 0.0f, 0.0f });
    xAxisArrow->transform.rotate(AngleAxisf(pi / 2.0f, Vector3f::UnitZ()));
    xAxisArrow->transform.scale(25.f);

    zAxisArrow = std::make_shared<Mesh>(*xAxisArrow);
    zAxisArrow->setVertexColor({ 0.0f, 0.0f, 1.0f });
    zAxisArrow->transform.rotate(AngleAxisf(pi / 2.0f, Vector3f::UnitX()));
    zAxisArrow->transform.scale(25.f);

    textL = std::make_shared<Mesh>(std::filesystem::path("resources/models/L.obj"), program);
    textA = std::make_shared<Mesh>(std::filesystem::path("resources/models/A.obj"), program);
    textB = std::make_shared<Mesh>(std::filesystem::path("resources/models/B.obj"), program);

    textL->transform.translate(Vector3f{ 0.0f, 200.0f, 0.0f });
    textL->transform.scale(50.0f);
    textL->transform.rotate(AngleAxisf(pi / 2.0f, Vector3f::UnitX()));
    textL->setVertexColor({ 1.0f, 1.0f, 1.0f });
    textA->transform.translate(Vector3f{ -200.0f, 0.0f, 0.0f });
    textA->transform.scale(50.0f);
    textA->setVertexColor({ 1.0f, 1.0f, 1.0f });
    textB->transform.translate(Vector3f{ 0.0f, 0.0f, 200.0f });
    textB->transform.scale(50.0f);
    textB->setVertexColor({ 1.0f, 1.0f, 1.0f });

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

    program.setUniform("uTView", cam.getView());
    program.setUniform("uTProj", cam.getProj());
}

void App::draw(float time, float delta)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) $glChk;
    program.use();
    for (auto& mesh : gamutMeshes) {
        mesh->draw(mesh->isWireframe);
    }
    xAxisArrow->draw();
    yAxisArrow->draw();
    zAxisArrow->draw();
    textL->draw();
    textA->draw();
    textB->draw();
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

void App::loadGamutMesh(const std::string& filepath)
{
    gamutMeshes.emplace_back(std::make_shared<Gamut::GamutMesh>(filepath, program));
    gamutMeshes.back()->transform.rotate(AngleAxisf(pi / 2.0f, Vector3f::UnitZ()));
}

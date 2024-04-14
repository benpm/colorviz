#include <app.hpp>
#include <filesystem>
App::App(Vector2f winSize)
{
    glfwSwapInterval(1);
    glEnable(GL_DEBUG_OUTPUT) $glChk;
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS) $glChk;
    glEnable(GL_DEPTH_TEST) $glChk;
    glFrontFace(GL_CCW) $glChk;
    glCullFace(GL_BACK) $glChk;
    glEnable(GL_BLEND) $glChk;
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) $glChk;

    program.compile({
        Shader(GL_VERTEX_SHADER, fs::path("resources/shaders/mesh.vert")),
        Shader(GL_FRAGMENT_SHADER, fs::path("resources/shaders/mesh.frag")),
    });

    this->transparentGamut = 0;
    this->gamutOpacity = 0.5f;

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

    // Find gamut files in resources/profiles
    for (const auto& entry : fs::directory_iterator("resources/profiles")) {
        if (entry.path().extension() == ".gam") {
            gamuts.push_back({ entry.path(), nullptr });
        }
    }

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
    ImGui_ImplOpenGL3_Init("#version 330");
    ImGui::StyleColorsDark();
}

void App::prepare()
{
    mouse.scroll = 0.0f;
    mouse.disabled = false;
}

void App::update(float time, float delta)
{
    updateGUI();

    mouse.disabled = ImGui::GetIO().WantCaptureMouse;

    mouse.delta = mouse.pos - mouse.lastPos;
    mouse.dragDelta = Vector2f(mouse.dragStart - mouse.pos) * (float)mouse.left * 2.0f;
    mouse.dragDelta.y() *= -1.0f;
    mouse.lastPos = mouse.pos;

    if (!mouse.disabled) {
        Vector2f screenDragDelta = mouse.delta * (float)mouse.left;
        camCtrl.control(
            screenDragDelta.cwiseQuotient(cam.viewSize),
            mouse.dragDelta.cwiseQuotient(cam.viewSize), { 0.0f, 0.0f }
        );
        camCtrl.universalZoom(mouse.scroll);
        camCtrl.update(cam, cam.viewSize);
    }

    // Smoothly switch between color spaces
    if (this->isAnimateSpace) {
        this->startTime = time;
        this->isAnimateSpace = false;
    }
    const float animationDuration = 1.0f;
    if (this->startTime >= 0.0f) {
        float t = time - this->startTime;
        float interpValue = t / animationDuration;
        if (interpValue > 1.0f) {
            this->startTime = -1.0f;
            this->spaceInterpolant = this->targetSpaceInterpolant;
        } else {
            this->spaceInterpolant = lerp(
                1.0f - this->targetSpaceInterpolant, this->targetSpaceInterpolant, interpValue
            );
        }
    }

    program.setUniform("uTView", cam.getView());
    program.setUniform("uTProj", cam.getProj());
    program.setUniform("uOpacity", 1.0f);
}

void App::draw(float time, float delta)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT) $glChk;
    program.use();
    program.setUniform("spaceInterp", 0.0f);
    xAxisArrow->draw();
    yAxisArrow->draw();
    zAxisArrow->draw();
    textL->draw();
    textA->draw();
    textB->draw();

    program.setUniform("spaceInterp", this->spaceInterpolant);
    for (int i = 0; i < gamuts.size(); i++) {
        if (i != transparentGamut && gamuts[i].mesh) {
            std::shared_ptr<Gamut::GamutMesh>& gamut = gamuts[i].mesh;
            float extent = (gamut->bbMax - gamut->bbMin).minCoeff();
            program.setUniform("uExtent", extent);
            gamut->draw(gamut->isWireframe);
        }
    }

    if (transparentGamut != -1 && gamuts[transparentGamut].mesh) {
        std::shared_ptr<Gamut::GamutMesh>& gamut = gamuts[transparentGamut].mesh;
        program.setUniform("uOpacity", gamutOpacity);
        float extent = (gamut->bbMax - gamut->bbMin).minCoeff();
        program.setUniform("uExtent", extent);
        gamut->draw(gamut->isWireframe);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::updateGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin(
            "Controls", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize
        )) {
        ImGui::SetWindowPos({ 4.0f, 4.0f }, ImGuiCond_FirstUseEver);
        if (ImGui::BeginCombo("Gamuts", "(select gamuts)")) {
            for (size_t i = 0; i < gamuts.size(); ++i) {
                if (ImGui::Selectable(
                        gamuts[i].path.stem().string().c_str(), (bool)gamuts[i].mesh
                    )) {
                    if (gamuts[i].mesh) {
                        gamuts[i].mesh.reset();
                    } else {
                        loadGamutMesh(i);
                    }
                }
            }
            ImGui::EndCombo();
        }
        if (ImGui::Button("Switch Colorspace")) {
            switchSpace();
        }
        ImGui::End();
    }

    // For each loaded gamut, show a window to control its settings
    for (size_t i = 0; i < gamuts.size(); ++i) {
        if (gamuts[i].mesh) {
            bool close = false;
            ImGui::Begin(
                gamuts[i].path.stem().string().c_str(), &close,
                ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize
            );

            ImGui::Checkbox("Wireframe", &gamuts[i].mesh->isWireframe);
            bool setTransparent = (transparentGamut == i);
            ImGui::Checkbox("Transparent", &setTransparent);
            transparentGamut = setTransparent ? i : transparentGamut;

            ImGui::End();
            if (close) {
                gamuts[i].mesh.reset();
            }
        }
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

void App::loadGamutMesh(size_t idx)
{
    gamuts[idx].mesh = std::make_shared<Gamut::GamutMesh>(gamuts[idx].path.string(), program);
    gamuts[idx].mesh->transform.rotate(AngleAxisf(pi / 2.0f, Vector3f::UnitZ()));
}

void App::switchSpace()
{
    this->targetSpaceInterpolant = 1.0f - this->spaceInterpolant;
    this->isAnimateSpace = true;
}

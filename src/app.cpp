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

    this->transparentGamut = -1;
    this->gamutOpacity = 0.5f;

    yAxisArrow =
        std::make_shared<Mesh>(std::filesystem::path("resources/models/arrow.obj"), program);
    yAxisArrow->setVertexColor({ 1.0f, 0.0f, 0.0f });
    yAxisArrow->transform.scale(25.f);

    xAxisArrow = std::make_shared<Mesh>(*yAxisArrow);
    xAxisArrow->setVertexColor({ 0.0f, 1.0f, 0.0f });
    xAxisArrow->transform.rotate(AngleAxisf(pi / 2.0f, Vector3f::UnitZ()));
    xAxisArrow->transform.scale(25.f);

    zAxisArrow = std::make_shared<Mesh>(*xAxisArrow);
    zAxisArrow->setVertexColor({ 0.0f, 0.0f, 1.0f });
    zAxisArrow->transform.rotate(AngleAxisf(pi / 2.0f, Vector3f::UnitX()));
    zAxisArrow->transform.scale(25.f);

    textL = std::make_shared<Mesh>(std::filesystem::path("resources/models/L.obj"), program);
    textA = std::make_shared<Mesh>(std::filesystem::path("resources/models/a.obj"), program);
    textB = std::make_shared<Mesh>(std::filesystem::path("resources/models/b.obj"), program);

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

    textR = std::make_shared<Mesh>(std::filesystem::path("resources/models/R.obj"), program);
    textG = std::make_shared<Mesh>(std::filesystem::path("resources/models/G.obj"), program);
    textBcaps =
        std::make_shared<Mesh>(std::filesystem::path("resources/models/bCaps.obj"), program);
    textR->transform = textL->transform;
    textR->setVertexColor({ 1.0f, 1.0f, 1.0f });
    textG->transform = textA->transform;
    textG->setVertexColor({ 1.0f, 1.0f, 1.0f });
    textBcaps->transform = textB->transform;
    textBcaps->setVertexColor({ 1.0f, 1.0f, 1.0f });

    camCtrl.mode = CameraControl::Mode::trackball;
    camCtrl.orbitDist(500.0f);
    cam.pos = { 0.0f, 0.0f, 2.0f };
    cam.viewSize = winSize;

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
    if (this->spaceInterpolant == 0.0f) {
        textL->draw();
        textA->draw();
        textB->draw();
    } else if (this->spaceInterpolant == 1.0f) {
        textR->draw();
        textG->draw();
        textBcaps->draw();
    }

    program.setUniform("spaceInterp", this->spaceInterpolant);
    for (int i = 0; i < gamuts.size(); i++) {
        if (i != transparentGamut) {

            gamuts[i]->draw(gamuts[i]->isWireframe);
        }
    }

    if (intersectionHash!=0) {
        program.setUniform("uOpacity", 1.0f);
        program.setUniform("spaceInterp", 0.0f);
        intersectionMeshes[intersectionHash]->draw();
    }

    if (transparentGamut != -1 && gamuts[transparentGamut]) {
        program.setUniform("uOpacity", gamutOpacity);
        gamuts[transparentGamut]->draw(gamuts[transparentGamut]->isWireframe);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void App::updateGUI()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if (ImGui::Begin("Controls", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) 

    ImGui::SetWindowPos({ 4.0f, 4.0f }, ImGuiCond_FirstUseEver);
    if (ImGui::BeginCombo("Gamuts", "(select gamuts)")) {
        std::vector<fs::path> gamutPaths;
        // Find gamut files in resources/profiles
        for (const auto& entry : fs::directory_iterator("resources/profiles")) {
            if (entry.path().extension() == ".gam") {
                gamutPaths.push_back(entry.path());
            }
        }
        for (size_t i = 0; i < gamutPaths.size(); ++i) {
            if (ImGui::Selectable(
                    gamutPaths[i].stem().string().c_str(), importedGamuts.contains(gamutPaths[i])
                )) {
                if (!importedGamuts.contains(gamutPaths[i])) {
                    importedGamuts.insert(gamutPaths[i]);
                    loadGamutMesh(gamutPaths[i]);
                }
            }
        }
        ImGui::EndCombo();
    }
    if (ImGui::Button("Switch Colorspace")) {
        switchSpace();
    }
    // if (gamuts[0].mesh && gamuts[1].mesh && ImGui::Button("Intersect")) {
    //     intersectGamutMeshes(gamuts[0].mesh, gamuts[1].mesh);
    // }
   
    

    if (ImGui::BeginTable(
            "Gamuts", 4,
            ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg
        )) {
        ImGui::TableSetupColumn("Visible", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Wireframe", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableSetupColumn("Transparent", ImGuiTableColumnFlags_WidthFixed, 100.0f);
        ImGui::TableHeadersRow();
        for (size_t i = 0; i < gamuts.size(); ++i) {           
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Checkbox(("##visible" + std::to_string(i)).c_str(), &gamuts[i]->isActive);
            ImGui::TableNextColumn();
            ImGui::Text(gamuts[i]->label.c_str());
            ImGui::TableNextColumn();
            bool prevWireframe = gamuts[i]->isWireframe;
            ImGui::Checkbox(
                ("##wireframe" + std::to_string(i)).c_str(), &gamuts[i]->isWireframe
            );
            ImGui::TableNextColumn();
            bool setTransparent = (transparentGamut == i);
            bool prevTransparent = setTransparent;
            ImGui::Checkbox(("##transparent" + std::to_string(i)).c_str(), &setTransparent);
            transparentGamut = setTransparent ? i : transparentGamut;
            if (prevTransparent && !setTransparent) {
                transparentGamut = -1;
            }            
        }
        ImGui::EndTable();
    }
    ImGui::End();
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

void App::loadGamutMesh(const fs::path& filepath)
{
    gamuts.push_back(std::make_shared<Gamut::GamutMesh>(filepath.string(), program));
    gamuts.back()->label = filepath.stem().string();
    gamuts.back()->transform.rotate(AngleAxisf(pi / 2.0f, Vector3f::UnitZ()));
    if (gamuts.size() == 1) {
        float extent = (gamuts[0]->bbMax - gamuts[0]->bbMin).minCoeff();
        program.use();
        program.setUniform("uExtent", extent);
    }
}

void App::switchSpace()
{
    this->targetSpaceInterpolant = 1.0f - this->spaceInterpolant;
    this->isAnimateSpace = true;
}

void App::intersectGamutMeshes(
    std::shared_ptr<Gamut::GamutMesh> a,
    std::shared_ptr<Gamut::GamutMesh> b
)
{
    SurfaceMesh mesh;
    bool result = PMP::corefine_and_compute_union(a->surfaceMesh, b->surfaceMesh, mesh);
    if (!result) {
        $warn("Intersection failed");
        return;
    }
    std::vector<Vector3f> vertices;
    std::vector<Vector3f> colors;
    for (const auto& v : mesh.vertices()) {
        Point3 p = mesh.point(v);
        vertices.push_back({ (float)p[0], (float)p[1], (float)p[2] });
        colors.push_back(Gamut::LABtoRGB(vertices.back()));
    }
    std::vector<Vector3u> faces;
    for (const auto& f : mesh.faces()) {
        Vector3u face;
        uint32_t i = 0;
        for (auto v : mesh.vertices_around_face(mesh.halfedge(f))) {
            face[i++] = v.idx();
        }
        faces.push_back(face);
    }
    intersectionMeshes[intersectionHash] = std::make_shared<Mesh>(vertices, faces, colors, program);
    intersectionMeshes[intersectionHash]->transform = a->transform;
}

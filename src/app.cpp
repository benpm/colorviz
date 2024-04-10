#include <app.hpp>

App::App(GLFWwindow* window)
{
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
    gfx::setbuf(GL_ELEMENT_ARRAY_BUFFER, ebo, data->triangles);
    program.setVertexAttrib(vbo, "vPos", 3, GL_FLOAT, 0u, 0u);
}

void App::prepare() {}

void App::update(float time, float delta)
{
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
    }
}

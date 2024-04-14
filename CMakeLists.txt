cmake_minimum_required(VERSION 3.20)
project(colorviz LANGUAGES CXX C)

# Dependency: spdlog
add_subdirectory(external/spdlog)

# Dependency: glfw
add_subdirectory(external/glfw)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_DOCS OFF)

# Dependency: glad
add_subdirectory(external/glad/cmake ${CMAKE_BINARY_DIR}/glad_bindir)
glad_add_library(glad_gl_core_4_5 STATIC REPRODUCIBLE API gl:core=4.5 EXTENSIONS GL_NV_shader_atomic_float)

# Dependency: gleq
include_directories(external/gleq)

# Dependency: eigen
find_package(Eigen3 REQUIRED)

# Dependency: imgui
file(GLOB IMGUI_SOURCES external/imgui/*.cpp)
add_library(imgui STATIC ${IMGUI_SOURCES}
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC external/imgui external/imgui/backends)
target_link_libraries(imgui PUBLIC glfw glad_gl_core_4_5)

# Dependency: OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})

# Dependency: tinyobjloader
add_subdirectory(external/tinyobjloader)

# Dependency: cgal
find_package(CGAL REQUIRED)

# Add sources, include dir, and create executable
file(GLOB_RECURSE SOURCES src/*.cpp src/*.c)
include_directories(include)
add_executable(app ${SOURCES})

target_compile_features(app PRIVATE cxx_std_20)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    message(STATUS "Clang detected, adding compile flags")
    target_compile_options(app PRIVATE -Wall -fconcepts-diagnostics-depth=3 -Wno-c++98-compat -Wno-reserved-id-macro)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    message(STATUS "GCC detected, adding compile flags")
    target_compile_options(app PRIVATE -Wall -fconcepts-diagnostics-depth=3)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    message(STATUS "msvc detected, adding compile flags")
    target_compile_options(app PRIVATE /std:c++20 /EHsc)
else()
    message(WARNING "Could not determine compiler: ${CMAKE_CXX_COMPILER_ID}")
    target_compile_options(app PRIVATE -Wall)
endif()

# Compile definitions
if(WIN32)
    message(STATUS "Windows detected, adding compile flags")
    target_compile_definitions(app PUBLIC
        PLATFORM_WINDOWS
        _USE_MATH_DEFINES
        NOMINMAX
    )
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "Debug build: adding compile flags")
    target_compile_definitions(app PRIVATE BUILD_DEBUG SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Release build: adding compile flags")
    target_compile_definitions(app PRIVATE BUILD_RELEASE SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_INFO)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_link_libraries(app PUBLIC
    glad_gl_core_4_5
    glfw
    ${OPENGL_LIBRARIES}
    spdlog
    Eigen3::Eigen
    imgui
    tinyobjloader
    CGAL::CGAL
)

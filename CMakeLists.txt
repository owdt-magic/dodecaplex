cmake_minimum_required(VERSION 3.19.4)
project("OWDT-OPENGL")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(BUILD_SHARED_LIBS ON)
find_package(PkgConfig)
find_package(glfw3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(assimp REQUIRED)

include_directories("./include/")

set(SHADER_DIR "${CMAKE_SOURCE_DIR}/shaders")
set(TEXTURE_DIR "${CMAKE_SOURCE_DIR}/textures")
set(ADD_SHADER_DIR "${CMAKE_SOURCE_DIR}/python/shader_scripts/programs")
set(GLOBAL_FONT "${CMAKE_SOURCE_DIR}/fonts/regular.ttf")
configure_file(config.h.in config.h)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${GLM_INCLUDE_DIRS})

message(STATUS "GLFW_LIBRARIES: ${GLFW_LIBRARIES}")
message(STATUS "OPENGL_LIBRARIES: ${OPENGL_LIBRARIES}")
message(STATUS "ASSIMP_LIBRARIES: ${ASSIMP_LIBRARIES}")
message(STATUS "GLM_LIBRARIES: ${GLM_LIBRARIES}")

file(GLOB SOURCES "src/*.cpp" "src/glad.c")
add_executable(game ${SOURCES}  main/game.cpp)
add_executable(spin ${SOURCES}  main/spin.cpp)
add_executable(vortex ${SOURCES} main/vortex.cpp)

include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.90.4
)
FetchContent_MakeAvailable(imgui)

# ImGui backends manually included
set(IMGUI_BACKENDS
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

add_executable(select
  main/select.cpp
  src/audio.cpp
  src/window.cpp
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${IMGUI_BACKENDS}
)

target_include_directories(select PRIVATE
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(select glfw ${OPENGL_LIBRARIES})
set(GLFW_LIBRARIES "/usr/local/Cellar/glfw/3.4/lib/libglfw.dylib")
set(COMMON_LIBS
    ${OPENGL_LIBRARIES}
    ${GLFW_LIBRARIES}
    glm::glm
    ${ASSIMP_LIBRARIES}
    -ldl
)
target_link_libraries(game ${COMMON_LIBS})
target_link_libraries(spin ${COMMON_LIBS})
target_link_libraries(vortex ${COMMON_LIBS})
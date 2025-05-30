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
add_executable(select src/audio.cpp main/select.cpp)

include(FetchContent)
FetchContent_Declare(
  raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 5.0
)
FetchContent_MakeAvailable(raylib)

target_link_libraries(select raylib)

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
#include "debug.h"

void checkGLError(const char* operation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after " << operation << ": " << error << std::endl;
    }
};

void checkOpenGLErrors() {
    // Check for OpenGL errors
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "OpenGL Error: ";
        switch (error) {
            case GL_INVALID_ENUM:
                std::cout << "GL_INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                std::cout << "GL_INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                std::cout << "GL_INVALID_OPERATION";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION";
                break;
            case GL_OUT_OF_MEMORY:
                std::cout << "GL_OUT_OF_MEMORY";
                break;
            default:
                std::cout << "Unknown error";
                break;
        }
        std::cout << std::endl;
    }
};


void printGPUCapabilities() {
    // OpenGL version
    const GLubyte* version = glGetString(GL_VERSION);
    std::cout << "OpenGL Version: " << version << std::endl;

    // GLSL version
    const GLubyte* glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    std::cout << "GLSL Version: " << glslVersion << std::endl;

    // Vertex texture units
    GLint vertexTextureUnits;
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &vertexTextureUnits);
    std::cout << "Vertex Texture Units: " << vertexTextureUnits << std::endl;

    // Check for required extensions
    std::cout << "Required Extensions:" << std::endl;
    const char* requiredExtensions[] = {
        "GL_ARB_vertex_shader",
        "GL_ARB_fragment_shader",
        "GL_ARB_texture_float",
        "GL_ARB_vertex_program",
        "GL_EXT_gpu_shader4"
    };

    for (const char* extension : requiredExtensions) {
        bool supported = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) != 0;
        std::cout << "  " << extension << ": " << (supported ? "Supported" : "Not Supported") << std::endl;
    }
};
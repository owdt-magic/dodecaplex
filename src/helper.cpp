#include "helper.h"

void checkGLError(const char* operation) {
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        std::cerr << "OpenGL error after " << operation << ": " << error << std::endl;
    }
};

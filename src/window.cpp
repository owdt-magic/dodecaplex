#include <iostream>
#include "window.h"

GLFWwindow* simplestWindow(unsigned int width, unsigned int height, const char* title){
        
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);

    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    Uniforms* uniforms = new Uniforms();
    uniforms->windWidth = width;
    uniforms->windHeight = height;
    glfwSetWindowUserPointer(window, uniforms);
    
    return window;
}

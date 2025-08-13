#include "graphicsPipe.h"

float GraphicsPipe::time = 0.0f;
float GraphicsPipe::previousTime = 0.0f;
int GraphicsPipe::frameCount = 0;

void GraphicsPipe::initHere(GLFWwindow* w) {
    window = w;
    window_uniforms = getUniforms(window);
    
    switch (type) {
        case PipeType::SPIN:
            shader_interface = new SpinPatterns(clas, window_uniforms);
            window_name = "DODECAPLEX";
            break;               
        case PipeType::FRAGMENT:
            shader_interface = new FragPatterns(clas, window_uniforms);
            window_name = "SPINNING DODECAPLEX";
            break;
        case PipeType::GAME:
            shader_interface = new GamePatterns(clas, window_uniforms);
            window_name = "FRAGMENT SHADER";
            break;
    }
    previousTime = glfwGetTime();
    frameCount = 0;
}

void GraphicsPipe::initWindowed() {
    GLFWwindow* window = initializeWindow(1024, 1024, window_name, 
                                clas.fullscreen, clas.monitorIndex);
    initHere(window);
    window = nullptr;
}

void GraphicsPipe::establishShaders() {
    shader_interface->compile();

    window_uniforms->last_time = glfwGetTime();    
    window_uniforms->loading = false;
    
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
}

void GraphicsPipe::renderNextFrame(bool swapBuffers) {
    if (window_uniforms->loading) establishShaders();

    time = glfwGetTime();
    window_uniforms->this_time = time;
    frameCount++;

    if (time - previousTime >= 1.0) {
        std::string fpsTitle = std::string(window_name) + " - FPS: " + std::to_string(frameCount);
        glfwSetWindowTitle(window, fpsTitle.c_str());
        frameCount = 0;
        previousTime = time;
    }
    
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    shader_interface->render();

    if (swapBuffers) {
        glfwSwapInterval(1);
        glfwSwapBuffers(window);        
    }
    glfwPollEvents();
    window_uniforms->last_time = time;
}

GraphicsPipe::~GraphicsPipe() {
    if (shader_interface) {
        delete shader_interface;
    }
} 
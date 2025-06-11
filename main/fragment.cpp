#include "config.h"
#include "gameWindow.h"
#include "shaderClass.h"
#include "bufferObjects.h"
#include <chrono>
#include <thread>
#include "debug.h"
#include "cla.h"

int main(int argc, char** argv) {
    CLAs clas = parse(argc, argv);

    GLFWwindow* window = initializeWindow(1024, 1024, "FRAGMENT", clas.fullscreen, clas.monitorIndex);

    std::string fragShaderPath = clas.shaderPath.empty()
        ? std::string(FRAG_SHADER_DIR) + "/purple-vortex.frag"
        : clas.shaderPath;
    std::cout << fragShaderPath << std::endl;

    ShaderProgram vortex_shader( FRAG_SHADER_DIR "/rect.vert", fragShaderPath, false);

    Uniforms* uniforms = getUniforms(window);
    
    float time;
    GLuint  U_RESOLUTION, U_MOUSE, U_SCROLL, U_TIME;

    VAO fullscreenQuad = rasterPipeVAO();

    static double previousTime = glfwGetTime();
    static int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        if (uniforms->loading) {
            vortex_shader.Load();
            vortex_shader.Activate();
            
            U_RESOLUTION  = glGetUniformLocation(vortex_shader.ID, "u_resolution");
            U_MOUSE       = glGetUniformLocation(vortex_shader.ID, "u_mouse");
            U_SCROLL      = glGetUniformLocation(vortex_shader.ID, "u_scroll");
            U_TIME        = glGetUniformLocation(vortex_shader.ID, "u_time");

            uniforms->last_time         = glfwGetTime();
            uniforms->loading           = false;
            
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
        }        
        time = glfwGetTime();
        uniforms->this_time = time;
        frameCount++;

        if (time - previousTime >= 1.0) {
            std::string fpsTitle = "FRAGMENT - FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(window, fpsTitle.c_str());
            frameCount = 0;
            previousTime = time;
        }


        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUniform2f(U_RESOLUTION,   uniforms->windWidth, \
                                    uniforms->windHeight);
        glUniform2f(U_MOUSE,        uniforms->mouseX, \
                                    uniforms->mouseY);
        glUniform1f(U_SCROLL,       uniforms->scroll);
        glUniform1f(U_TIME, time);

        fullscreenQuad.DrawElements(GL_TRIANGLES);
        
        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        glfwPollEvents();
        uniforms->last_time = time;
        //checkGLError("At end of loop...");
    }
    return 0;
}
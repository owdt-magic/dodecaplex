#include "audio.h"
#include "config.h"
#include "gameWindow.h"
#include "shaderClass.h"
#include "bufferObjects.h"
#include "models.h"
#include <chrono>
#include <thread>
#include "debug.h"
#include "cla.h"
#include "sharedUniforms.h"

int main(int argc, char** argv) {
    CLAs clas = parse(argc, argv);

    GLFWwindow* window = initializeWindow(1024, 1024, "SPINNING DODECAPLEX", clas.fullscreen, clas.monitorIndex);

    AudioNest audio_nest(clas.audioIndex);

    ShaderProgram world_shader(     SHADER_DIR "/world.vert", \
                                    SHADER_DIR "/prune.geom", \
                                    SHADER_DIR "/spin.frag", false);
    
    PlayerContext player_context;
    player_context.initializeMapData();
    //TextureLibrary texture_library;

    Uniforms* uniforms = getUniforms(window);
    
    SharedUniforms shared_uniforms(false);


    float time;
    GLuint U_RESOLUTION, U_MOUSE, U_SCROLL, U_TIME, U_BANDS, U_SCALE,U_BRIGHTNESS, U_HUESHIFT;

    CameraInfo cam;

    player_context.populateDodecaplexVAO(RhombusPattern(WebType::DOUBLE_STAR, false));
    
    GLuint U_GLOBAL;
    glGenBuffers(1, &U_GLOBAL);
    glBindBuffer(GL_UNIFORM_BUFFER, U_GLOBAL);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, U_GLOBAL);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    
    static double previousTime = glfwGetTime();
    static int frameCount = 0;

    while (!glfwWindowShouldClose(window)) {
        if (uniforms->loading) {
            world_shader.Load();
            world_shader.Activate();
            
            //texture_library.linkPentagonLibrary(world_shader.ID);

            U_RESOLUTION  = glGetUniformLocation(world_shader.ID, "u_resolution");
            U_MOUSE       = glGetUniformLocation(world_shader.ID, "u_mouse");
            U_SCROLL      = glGetUniformLocation(world_shader.ID, "u_scroll");
            U_TIME        = glGetUniformLocation(world_shader.ID, "u_time");
            U_BANDS       = glGetUniformLocation(world_shader.ID, "u_audio_bands");
            U_BRIGHTNESS  = glGetUniformLocation(world_shader.ID, "u_brightness");
            U_SCALE       = glGetUniformLocation(world_shader.ID, "u_scale");
            U_HUESHIFT    = glGetUniformLocation(world_shader.ID, "u_hueShift");
            
            uniforms->last_time         = glfwGetTime();
            uniforms->loading           = false;
            uniforms->player_context    = &player_context;
            
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
        }        
        time = glfwGetTime();
        uniforms->this_time = time;
        frameCount++;

        if (time - previousTime >= 1.0) {
            std::string fpsTitle = "SPINING DODECAPLEX - FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(window, fpsTitle.c_str());
            frameCount = 0;
            previousTime = time;
        }

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        accountSpin(uniforms, cam, shared_uniforms.data->speed, shared_uniforms.data->fov);

        glBufferSubData(GL_UNIFORM_BUFFER, 0,                 sizeof(glm::mat4), &(cam.Projection)[0][0]);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &(cam.Model)[0][0]);

        world_shader.Activate();

        glUniform2f(U_RESOLUTION,   uniforms->windWidth, \
                                    uniforms->windHeight);
        glUniform2f(U_MOUSE,        uniforms->mouseX, \
                                    uniforms->mouseY);
        glUniform1f(U_SCROLL,       uniforms->scroll);
        glUniform1f(U_TIME, time);
        
        audio_nest.processFFT();

        glUniform4f(U_BANDS, audio_nest.g_bandAmplitudes[0],
                             audio_nest.g_bandAmplitudes[1],
                             audio_nest.g_bandAmplitudes[2],
                             audio_nest.g_bandAmplitudes[3]);

        glUniform1f(U_BRIGHTNESS, shared_uniforms.data->brightness);
        glUniform1f(U_SCALE,      shared_uniforms.data->scale);
        glUniform1f(U_HUESHIFT,   shared_uniforms.data->hueShift);

        player_context.drawMainVAO();

        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        glfwPollEvents();
        uniforms->last_time = time;
        //checkGLError("At end of loop...");
    }
    return 0;
}
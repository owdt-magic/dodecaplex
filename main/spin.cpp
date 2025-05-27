#include "audio.h"
#include "config.h"
#include "window.h"
#include "shaderClass.h"
#include "bufferObjects.h"
#include "models.h"
#include <chrono>
#include <thread>
#include "debug.h"

constexpr int BUFFER_SIZE = 2048;

std::vector<float> g_audioBuffer(BUFFER_SIZE);
std::atomic<float> g_bandAmplitudes[4] = {0.0f, 0.0f, 0.0f, 0.0f};

void data_callback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
    static size_t g_writeHead = 0;
    const float* in = (const float*)input;
    for (ma_uint32 i = 0; i < frameCount && g_writeHead < BUFFER_SIZE; ++i) {
        g_audioBuffer[g_writeHead++] = in[i]; // mono
    }
    if (g_writeHead >= BUFFER_SIZE) {
        g_writeHead = 0;
    }
}

int main() {
    GLFWwindow* window = initializeWindow(1024, 1024, "DODECAPLEX");

    //--------- vvv MiniAudio Context vvv --------
        ma_context context;
        ma_result result = ma_context_init(NULL, 0, NULL, &context);
        if (result != MA_SUCCESS) {
            std::cerr << "Failed to initialize miniaudio context.\n";
            return -1;
        }

        ma_device_id selectedInputDevice = select_input_device(&context);

        ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);
        deviceConfig.capture.pDeviceID = &selectedInputDevice;
        deviceConfig.capture.format   = ma_format_f32;
        deviceConfig.capture.channels = 1;
        deviceConfig.sampleRate       = SAMPLE_RATE;
        deviceConfig.dataCallback     = data_callback;

        ma_device device;
        if (ma_device_init(&context, &deviceConfig, &device) != MA_SUCCESS) {
            std::cerr << "Failed to initialize audio device.\n";
            return -1;
        }
        ma_device_start(&device);
    //--------- ^^^ MiniAudio Context ^^^ --------

    ShaderProgram world_shader(     SHADER_DIR "/world.vert", \
                                    SHADER_DIR "/prune.geom", \
                                    SHADER_DIR "/spin.frag", false);
    
    PlayerContext player_context;
    player_context.initializeMapData();

    Uniforms* uniforms = getUniforms(window);
    
    float time;
    GLuint U_RESOLUTION, U_MOUSE, U_SCROLL, U_TIME, U_BANDS;

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

            U_RESOLUTION  = glGetUniformLocation(world_shader.ID, "u_resolution");
            U_MOUSE       = glGetUniformLocation(world_shader.ID, "u_mouse");
            U_SCROLL      = glGetUniformLocation(world_shader.ID, "u_scroll");
            U_TIME        = glGetUniformLocation(world_shader.ID, "u_time");
            U_BANDS       = glGetUniformLocation(world_shader.ID, "u_audio_bands");
            
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
            std::string fpsTitle = "VORTEX - FPS: " + std::to_string(frameCount);
            glfwSetWindowTitle(window, fpsTitle.c_str());
            frameCount = 0;
            previousTime = time;
        }

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        accountSpin(uniforms, cam);

        glBufferSubData(GL_UNIFORM_BUFFER, 0,                 sizeof(glm::mat4), &(cam.Projection)[0][0]);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &(cam.Model)[0][0]);

        world_shader.Activate();

        glUniform2f(U_RESOLUTION,   uniforms->windWidth, \
                                    uniforms->windHeight);
        glUniform2f(U_MOUSE,        uniforms->mouseX, \
                                    uniforms->mouseY);
        glUniform1f(U_SCROLL,       uniforms->scroll);
        glUniform1f(U_TIME, time);

        processFFT(g_audioBuffer, g_bandAmplitudes);

        glUniform4f(U_BANDS, g_bandAmplitudes[0],
                             g_bandAmplitudes[1],
                             g_bandAmplitudes[2],
                             g_bandAmplitudes[3]);

        player_context.drawMainVAO();

        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        glfwPollEvents();
        uniforms->last_time = time;
        //checkGLError("At end of loop...");
    }
    return 0;
}
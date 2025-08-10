#ifndef WINDOW_H
#define WINDOW_H
#include "world.h"

#include <GLFW/glfw3.h>
#include <array>


struct Uniforms
{
    unsigned int windWidth, windHeight;
    float mouseX, mouseY;
    float scroll = 1.0;
    bool loading = true;
    PlayerContext* player_context;
    bool key_states[1024] = {false};
    bool click_states[3] = {false};
    float click_times[3] = {0.0f};
    std::array<bool, 4> getWASD();
    float last_time = 0.0f;
    float this_time = 0.0f;
};
Uniforms* getUniforms(GLFWwindow* window);

GLFWwindow* simplestWindow(unsigned int start_width, unsigned int start_height, const char* title);

#endif
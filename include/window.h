#ifndef WINDOW_H
#define WINDOW_H

#include "spells.h"
#include "dodecaplex.h"
#include <array>
#include <deque>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>

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

struct CameraInfo
{
    glm::mat4 Projection;
    glm::mat4 View;
    glm::mat4 Model;
    glm::vec3 Location;
};
void accountCameraControls(Uniforms* uniforms, CameraInfo& camera_mats);
GLuint getSpellSubroutine(Uniforms* uniforms, Grimoire& grimoire, GLuint shader_id);

GLFWwindow* initializeWindow(unsigned int start_width, unsigned int start_height, const char* title);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

#endif
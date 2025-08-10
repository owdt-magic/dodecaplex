#include "spells.h"
#include "window.h"
#include <functional>
#include <array>
#include <deque>
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

struct CameraInfo
{
    glm::mat4 Projection;
    glm::mat4 View;
    glm::mat4 Model;
    glm::vec3 Location;
};

GLFWwindow* initializeWindow(unsigned int start_width, unsigned int start_height, const char* title, bool fullscreen, int monitorIndex);
GLFWwindow* initializeWindow(unsigned int start_width, unsigned int start_height, const char* title);

void accountCameraControls(Uniforms* uniforms, CameraInfo& camera_mats);
void accountSpin(Uniforms* uniforms, CameraInfo& camera_mats);
void accountSpin(Uniforms* uniforms, CameraInfo& camera_mats, float scale, float warp, float scroll);

GLuint getSpellSubroutine(Uniforms* uniforms, Grimoire& grimoire, GLuint shader_id);
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void resizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

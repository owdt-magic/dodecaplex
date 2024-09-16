#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

void checkGLError(const char* operation);
void checkOpenGLErrors();
void printGPUCapabilities();
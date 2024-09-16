#include <iostream>
#include <glad/glad.h>
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/string_cast.hpp"

void checkGLError(const char* operation);
void checkOpenGLErrors();
void printGPUCapabilities();
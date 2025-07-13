#include "debug.h"
#include "gameWindow.h"

bool window_is_focused = false;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Uniforms* uniforms = getUniforms(window);

    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_ESCAPE) {
            glfwSetWindowShouldClose(window, true);
        } else if (key == GLFW_KEY_SPACE) {
            uniforms->loading = true;
            std::cout << "Reloading Shaders" << std::endl;            
        } else if (key == GLFW_KEY_TAB) {
            if (window_is_focused) {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                window_is_focused = false;
            } else {
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                window_is_focused = true;
            }
        }
        uniforms->key_states[key] = true;
    } else if (action == GLFW_RELEASE) {
        uniforms->key_states[key] = false;
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Uniforms* uniforms = getUniforms(window);
    if (button < 3) { // AKA, left. right, middle
        if (action == GLFW_PRESS) {
            uniforms->click_states[button] = true;
            uniforms->click_times[button] = glfwGetTime();
        } else if (action == GLFW_RELEASE) {
            uniforms->click_states[button] = false;
        }
    }
}

void resizeCallback(GLFWwindow* window, int width, int height) {
    Uniforms* uniforms = getUniforms(window);
    uniforms->windWidth = width;
    uniforms->windHeight = height;
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    Uniforms* uniforms = getUniforms(window);

    uniforms->mouseX = float(xpos) / float(uniforms->windWidth);
    uniforms->mouseY = float(ypos) / float(uniforms->windHeight);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Uniforms* uniforms = getUniforms(window);
    uniforms->scroll += float(yoffset/10.0);
}

GLFWwindow* initializeWindow(unsigned int width, unsigned int height, const char* title, bool fullscreen, int monitorIndex) {
    GLFWwindow* window = simplestWindow(width, height, title);

    // Always create windowed, and apply borderless fullscreen manually if needed
    glfwWindowHint(GLFW_DECORATED, fullscreen ? GLFW_FALSE : GLFW_TRUE);

    if (fullscreen) {
        int monitorCount;
        GLFWmonitor** monitors = glfwGetMonitors(&monitorCount);
        if (monitorIndex < 0 || monitorIndex >= monitorCount) monitorIndex = 0;

        GLFWmonitor* monitor = monitors[monitorIndex];
        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        int xpos, ypos;
        glfwGetMonitorPos(monitor, &xpos, &ypos);
        glfwSetWindowAttrib(window, GLFW_DECORATED, GLFW_FALSE);
        glfwSetWindowPos(window, xpos, ypos);
        glfwSetWindowSize(window, mode->width, mode->height);
    }

    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, width, height);

    int system_width, system_height;
    glfwGetFramebufferSize(window, &system_width, &system_height);
    glViewport(0, 0, system_width, system_height);

    // GLAD loader check
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return nullptr;
    }

    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetFramebufferSizeCallback(window, resizeCallback);

    Uniforms* uniforms = new Uniforms();
    uniforms->windWidth = system_width;
    uniforms->windHeight = system_height;
    glfwSetWindowUserPointer(window, uniforms);

    if (window_is_focused)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    printGPUCapabilities();
    return window;
}

GLFWwindow* initializeWindow(unsigned int start_width, unsigned int start_height, const char* title) {
    return initializeWindow(start_width, start_height, title, false, 999);
}


void accountCameraControls(Uniforms* uniforms, CameraInfo &camera_info) {
    PlayerContext* player_context = uniforms->player_context;
    float ratio = float(uniforms->windWidth)/float(uniforms->windHeight);
    float dt = std::min( float(uniforms->this_time-uniforms->last_time), 0.01f);
    
    player_context->player_location->noclip = !uniforms->key_states[340];
        // Debugging: hold shift for free fly
    camera_info.Model       = player_context->getModelMatrix(uniforms->getWASD(), uniforms->mouseX, uniforms->mouseY, dt);
    camera_info.Projection  = glm::perspective(glm::radians(110.0f), ratio, 0.01f, 100.0f);
    // Projection matrix: 90° Field of View, display range: 0.1 unit <-> 100 units
}
void accountSpin(Uniforms* uniforms, CameraInfo &camera_info) {
    accountSpin(uniforms, camera_info, 1.0f, 150.0f, 0.0f);
}
void accountSpin(Uniforms* uniforms, CameraInfo &camera_info, float scale, float fov, float scroll) {
    const float ds = rand();
    static glm::mat4 rotation = glm::mat4({
        1.0f,  0.0f,    0.0f, 0.0f,
        0.0f,  cos(ds), 0.0f, sin(ds),
        0.0f,  0.0f,    1.0f, 0.0f,
        0.0f, -sin(ds), 0.0f, cos(ds)
    }); // provides variance and avoids horizontal line aliasing
    float ratio = float(uniforms->windWidth)/float(uniforms->windHeight);
    float dt = std::min( float(uniforms->this_time-uniforms->last_time), 0.01f);    
    scroll+=uniforms->scroll;
    float dx, dy;
    dx = dt*sin(scroll/10.0f)*scale;
    dy = dt*cos(scroll/10.0f)*scale;
    rotation *= glm::mat4({
        cos(dx), 0.0f, 0.0f, sin(dx), 
        0.0f,     1.0f, 0.0f, 0.0f, 
        0.0f,     0.0f, 1.0f, 0.0f,     
       -sin(dx), 0.0f, 0.0f, cos(dx)
    });
    rotation *= glm::mat4({
        1.0f, 0.0f,  0.0f,     0.0f, 
        0.0f, 1.0f,  0.0f,     0.0f, 
        0.0f, 0.0f,  cos(dy), sin(dy), 
        0.0f, 0.0f, -sin(dy), cos(dy)
    });
    camera_info.Model       = rotation;
    camera_info.Projection  = glm::perspective(glm::radians(fov), ratio, 0.01f, 100.0f);
}

GLuint getSpellSubroutine(Uniforms* uniforms, Grimoire& grimoire, GLuint shader_id) {
    static GLuint subroutine_index = 0;    
    float current_time = glfwGetTime();
    if (uniforms->click_states[0] && !grimoire.active_spell->spell_life && !grimoire.flipping()) {
        // The mouse is being held down... AND the spell is not currently running.
        if(!grimoire.active_spell->click_time) {
            subroutine_index = glGetSubroutineIndex(shader_id, GL_FRAGMENT_SHADER, 
                                                    grimoire.active_spell->cast_subroutine);
            // And it's the first frame of it being held down...
            grimoire.active_spell->click_time = current_time;
        }
        grimoire.chargeSpell(current_time, uniforms->player_context);
    } else if (grimoire.active_spell->click_time) {        
        subroutine_index = glGetSubroutineIndex(shader_id, GL_FRAGMENT_SHADER,
                                                grimoire.active_spell->release_subroutine);
        // The mouse was JUST released
        grimoire.startSpell(current_time, uniforms->player_context);
    } else if (grimoire.active_spell->spell_life) {
        // The spell has been cast, and will decay from 1.0f to 0.0f
        // If its at 0.0f this will not be triggered..
        grimoire.updateSpellLife(current_time, uniforms->player_context);
        if (grimoire.active_spell->spell_life == 0.0f) {
            subroutine_index = glGetSubroutineIndex(shader_id, GL_FRAGMENT_SHADER, "emptySpell");
            // The spell is complete, we change the subroutine for the last pass..
            grimoire.updateSpellLife(current_time, uniforms->player_context);
            //And ensure that 0.0f is passed so that cleanup can occur.
        }
    } else {
        // This is the condition where the player can change pages of their grimoire and
        // not interfere with their spells.
        if (uniforms->key_states[GLFW_KEY_E]) {
            grimoire.flipRight(current_time);
        } else if (uniforms->key_states[GLFW_KEY_Q]) {
            grimoire.flipLeft(current_time);
        } else if ( abs(current_time-grimoire.flip_start) < grimoire.flip_durration*1.1f) {
            grimoire.updateFlip(current_time);
        }
    }
    return subroutine_index;
}


Uniforms* getUniforms(GLFWwindow* window) {
    return static_cast<Uniforms*>(glfwGetWindowUserPointer(window));
}

std::array<bool, 4> Uniforms::getWASD() { 
    return {key_states[GLFW_KEY_W], key_states[GLFW_KEY_A], 
            key_states[GLFW_KEY_S], key_states[GLFW_KEY_D]};
}

#ifndef WORLD_H
#define WORLD_H

#include "cell.h"
#include "bufferObjects.h"
#include "dodecaplex.h"
#include "transform.h"
#include <time.h>
#include <vector>
#include <cmath>
#include <glm/gtc/type_ptr.hpp>

struct InterceptResult {
    WorldCell* cell;
    glm::vec3 point;
    int index;
};

struct PlayerLocation {    
    PlayerLocation();
    glm::mat4 getModel();
    glm::vec3 getFocus();
    glm::vec3 getHead();
    glm::vec3 getPUp();
    float getHeight();
    InterceptResult getIntercept();
    void teleportHead(glm::vec3 target);
    void teleportPUp(glm::vec3 target);
    WorldCell* reference_cell = NULL;
    void updateFocus(float x, float y, float dt);
    void updatePosition(std::array<bool, 4> WASD, float dt);
private:
    float mx = 0.0f;
    float my = 0.0f;
    const float height = 1.0f;
    const float movement_scale = 0.5f;
    const float mouse_scale = 500.0f;

    glm::vec3 head = glm::vec3(0,0.000001,0);
    glm::vec3 player_up = glm::vec3(0,0,-1);
    glm::vec3 focus = glm::vec3(1,0,0);
        //NOTE: feet/focus are relative to the head!
        //i.e. the actuall coordinates of feet/focus:
        //head + feet / head + focus
    glm::mat4 orientation_transform = glm::mat4(1.0f);
    glm::mat4 displacement_transform = glm::mat4(1.0f);
    glm::mat4 accumulated_transforms = glm::mat4(1.0f);
};

struct PlayerContext {
    void linkDodecaplexVAOs();
    void drawAllVAOs();
    PlayerLocation* player_location = NULL;
    PlayerContext();
private:
    std::size_t initializeDodecaplexStates(GLuint* index_buffer);
    std::vector<VAO> all_vaos;
};


#endif
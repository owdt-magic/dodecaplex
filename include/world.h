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
    glm::mat4 getView(float x, float y, float dt);
    glm::mat4 getModel(std::array<bool, 4> WASD, float dt);
    glm::vec3 getFocus();
    glm::vec3 getHead();
    glm::vec3 getPUp();
    float getHeight();
    void setFloorIndex(int index);
    InterceptResult getIntercept();
    void teleportHead(glm::vec3 target);
    void teleportPUp(glm::vec3 target);
    WorldCell* reference_cell = NULL;
    
private:
    float mx = 0.0f;
    float my = 0.0f;
    const float height = 1.0f;
    const float movement_scale = 0.75f;
    const float mouse_scale = 500.0f;
    uint floor_indx;

    glm::vec3 head = glm::vec3(0,0.1,0);
    glm::vec3 player_up = glm::vec3(0,0,-1);
    glm::vec3 focus = glm::vec3(1,0,0);
        //NOTE: feet/focus are relative to the head!
        //i.e. the actuall coordinates of feet/focus:
        //head + feet / head + focus
    uint getFloorIndex();
    bool accountBoundary(glm::vec3& direction);
    void updateFocus(float x, float y, float dt);
    void moveHead(std::array<bool, 4> WASD, float dt);
};

struct PlayerContext {
    void linkPlayerCellVAOs();
    void drawPlayerCellVAOs();
    void linkDodecaplexVAOs();
    void drawAllVAOs();
    PlayerLocation* player_location = NULL;
    PlayerContext();
private:
    std::vector<WorldCell*> establishNeighborhood();
    std::vector<VAO> all_vaos;
};


#endif
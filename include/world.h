#ifndef WORLD_H
#define WORLD_H

#include "cell.h"
#include "bufferObjects.h"
#include <vector>


struct PlayerLocation {    
    PlayerLocation();
    glm::mat4 getView(float x, float y);
    glm::mat4 getModel(std::array<bool, 4> WASD);
    WorldCell* reference_cell = NULL;
    
private:
    float mx = 0.0f;
    float my = 0.0f;
    float height = 0.333f;//... seems good to me
    float movement_scale = 1.0f/5000.0f;
    uint floor_indx;

    glm::vec3 head = glm::vec3(0,0,0);
    glm::vec3 player_up = glm::vec3(0,0,-1);
    glm::vec3 focus = glm::vec3(1,0,0);
        //NOTE: feet/focus are relative to the head!
        //i.e. the actuall coordinates of feet/focus:
        //head + feet / head + focus
    uint getFloorIndex();
    void accountBoundary();
    void updateFocus(float x, float y);
};

struct PlayerContext {
    void linkPlayerCellVAOs();
    void drawPlayerCellVAOs();
    PlayerLocation* player_location = NULL;
    PlayerContext();
private:
    std::vector<WorldCell*> establishNeighborhood();
    std::vector<VAO> all_vaos;
};


#endif
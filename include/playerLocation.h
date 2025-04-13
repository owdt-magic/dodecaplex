#ifndef PLAYERLOCATION_H
#define PLAYERLOCATION_H

#include "config.h"
#include "dodecaplex.h"

struct AnimationInfo {
    glm::vec3 rotations;
    glm::vec3 offsets;
    float progress;
    float durration;
    AnimationInfo(
        glm::vec3 rs, glm::vec3 os, float p, float d
    ) : rotations(rs), offsets(os), progress(p), durration(d) {};
};

struct PlayerLocation {    
    PlayerLocation();
    glm::mat4 getModel(bool* loaded_cells);
    glm::mat4 currentTransform();
    glm::vec3 getFocus();
    glm::vec3 getHead();
    glm::vec3 getPUp();
    int getCellIndex();
    float getHeight();
    void teleportHead(glm::vec3 target);
    void teleportPUp(glm::vec3 target);
    void updateFocus(glm::vec3 rotations);
    void focusFromMouse(float x, float y, float dt);
    void updatePosition(glm::vec3 displacements);
    void positionFromKeys(std::array<bool, 4> WASD, float dt);
    glm::mat4 getGravity(glm::mat4 current_transform);
    void setAnimation(AnimationInfo* changes);
    glm::mat4 elapseAnimation(float dt);
    bool noclip = false;
    bool overridden = false;
private:
    float mx = 0.0f;
    float my = 0.0f;
    const float height = -3.1415f/20.0f;
    const float movement_scale = 0.5f;
    const float mouse_scale = MOUSE_SCALE;

    int cell_index = 0;
    int floor_index = 0;
    int centroid_index = neighbor_side_orders[cell_index*12+floor_index];
    glm::vec4 floor_centroid = dodecaplex_centroids[centroid_index];
    glm::vec4 cell_centroid = glm::vec4(0.0f,0.0f,0.0f,1.0f);
    
    glm::vec3 head = glm::vec3(0,0.000001,0);
    glm::vec3 player_up = glm::vec3(0,0,-1);
    glm::vec3 focus = glm::vec3(1,0,0);
    
    AnimationInfo* animation_info = NULL;
    glm::mat4 pitch_transform = glm::mat4(1.0f);
    glm::mat4 yaw_transform = glm::mat4(1.0f);
    glm::mat4 roll_transform = glm::mat4(1.0f);
    glm::mat4 displacement_transform = glm::mat4(1.0f);
    glm::mat4 accumulated_transforms = glm::mat4(1.0f);
    
};

#endif
#ifndef WORLD_H
#define WORLD_H

#include "cell.h"
#include "bufferObjects.h"
#include "dodecaplex.h"
#include <time.h>
#include <vector>
#include <cmath>
#include <set>
#include <glm/gtc/type_ptr.hpp>

struct InterceptResult {
    WorldCell* cell;
    glm::vec3 point;
    int index;
};

struct MapData {
    bool load_cell[120] = {false};
    void establishMap();
};

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
    glm::mat4 getModel(MapData map_data);
    glm::vec3 getFocus();
    glm::vec3 getHead();
    glm::vec3 getPUp();
    int getCellIndex();
    float getHeight();
    InterceptResult getIntercept();
    void teleportHead(glm::vec3 target);
    void teleportPUp(glm::vec3 target);
    WorldCell* reference_cell = NULL;
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
    const float mouse_scale = 500.0f;

    int cell_index = 0;
    int floor_index = 0;
    int centroid_index = neighbor_side_orders[cell_index*12+floor_index];
    glm::vec4 floor_centroid = 
        glm::vec4(  dodecaplex_centroids[centroid_index*4],
                    dodecaplex_centroids[centroid_index*4+1],
                    dodecaplex_centroids[centroid_index*4+2],
                    dodecaplex_centroids[centroid_index*4+3]);
    glm::vec4 cell_centroid = glm::vec4(0.0f,0.0f,0.0f,1.0f);
    
    glm::vec3 head = glm::vec3(0,0.000001,0);
    glm::vec3 player_up = glm::vec3(0,0,-1);
    glm::vec3 focus = glm::vec3(1,0,0);
        //NOTE: feet/focus are relative to the head!
        //i.e. the actuall coordinates of feet/focus:
        //head + feet / head + focus
    AnimationInfo* animation_info = NULL;
    glm::mat4 pitch_transform = glm::mat4(1.0f);
    glm::mat4 yaw_transform = glm::mat4(1.0f);
    glm::mat4 roll_transform = glm::mat4(1.0f);
    glm::mat4 displacement_transform = glm::mat4(1.0f);
    glm::mat4 accumulated_transforms = glm::mat4(1.0f);
    
};

struct PlayerContext {
    PlayerContext();
    void initializeWorldData();
    void indexDodecaplexVAOs();
    void drawAllVAOs();
    
    glm::mat4 getModelMatrix(std::array<bool, 4> WASD, float mouseX, float mouseY, float dt);
    PlayerLocation* player_location = NULL;
    MapData map_data;
private:
    VAO dodecaplex_vao;
    std::size_t populateDodecaplexIndexBuffer(GLuint* index_buffer);
    std::vector<VAO> additional_vaos;
};


#endif
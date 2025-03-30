#ifndef WORLD_H
#define WORLD_H

#include "triangularization.h"
#include "playerLocation.h"

#include <time.h>
#include <vector>
#include <tuple>
#include <set>
#include <functional>
#include <glm/gtc/type_ptr.hpp>

struct SubSurface {
    int num_faces;
    int* indeces_ptr; // This points into the side_indeces in MapData bellow
    SubSurface(int f, int* i) : num_faces(f), indeces_ptr(i) {};
};

struct MapData {
    void establishMap();
    std::vector<SubSurface> interior_surfaces;
    std::vector<SubSurface> adjacent_surfaces;
    bool load_cell[120] = {false};
    PentagonMemory pentagon_summary[120*12];
    bool load_side[120*12] = {false};
private:    
    std::vector<int> side_indeces; // content range: 0-120*12
    
};

struct PlayerContext {
    PlayerContext();
    ~PlayerContext();
    void initializeMapData();
    void populateDodecaplexVAO();
    void drawAllVAOs(GLuint U_WORLD);
    void updateOldPentagon(int map_index);
    void elapseShrapnel(float progress);
    glm::mat4 getModelMatrix(std::array<bool, 4> WASD, float mouseX, float mouseY, float dt);
    void spawnShrapnel(int map_index);
    PlayerLocation* player_location = NULL;
    MapData map_data;
private:
    glm::mat4 shrapnel_scatter = glm::mat4(1.0f);
    CPUBufferPair dodecaplex_buffers;
    VAO dodecaplex_vao;
    std::vector<VAO> additional_vaos;
};


#endif
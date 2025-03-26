#ifndef WORLD_H
#define WORLD_H

#include "bufferObjects.h"
#include "playerLocation.h"

#include <time.h>
#include <vector>
#include <tuple>
#include <set>
#include <functional>
#include <glm/gtc/type_ptr.hpp>

struct PentagonMemory {
    int v_index, i_index, size, source;
    PentagonMemory(int v, int i, int s, int src) : v_index(v), i_index(i), size(s), source(src) {};
    PentagonMemory() : v_index(0), i_index(0), size(0), source(0) {};
};

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
private:    
    std::vector<int> side_indeces; // content range: 0-120*12
    bool load_side[120*12] = {false};    
};

struct PlayerContext {
    PlayerContext();
    ~PlayerContext();
    void initializeMapData();
    void populateDodecaplexVAO();
    void drawAllVAOs();

    glm::mat4 getModelMatrix(std::array<bool, 4> WASD, float mouseX, float mouseY, float dt);
    PlayerLocation* player_location = NULL;
    MapData map_data;
private:
    size_t vertex_max_size;
    size_t index_max_size;    
    GLfloat* vertex_buffer;
    GLuint* index_buffer;
    VAO dodecaplex_vao;
    std::vector<VAO> additional_vaos;
};


#endif
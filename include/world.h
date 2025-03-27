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
    int v_start, v_end, i_start, i_end, i_offset, source;
    int v_len, i_len;

    PentagonMemory() {};
    PentagonMemory(int src) : source(src) {};
    void markStart(CPUBufferPair& bw);
    void markEnd(CPUBufferPair& bw);
    void unpack();
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
    bool load_side[120*12] = {false};
private:    
    std::vector<int> side_indeces; // content range: 0-120*12
    
};

struct PlayerContext {
    PlayerContext();
    ~PlayerContext();
    void initializeMapData();
    void populateDodecaplexVAO();
    void drawAllVAOs();
    void updateOldPentagon(PentagonMemory memory);
    glm::mat4 getModelMatrix(std::array<bool, 4> WASD, float mouseX, float mouseY, float dt);
    PlayerLocation* player_location = NULL;
    MapData map_data;
private:
    CPUBufferPair dodecaplex_buffers;
    VAO dodecaplex_vao;
    std::vector<VAO> additional_vaos;
};


#endif
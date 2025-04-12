#ifndef WORLD_H
#define WORLD_H

#include "rhombus.h"
#include "playerLocation.h"

#include <time.h>
#include <vector>
#include <tuple>
#include <set>
#include <map>
#include <functional>
#include <glm/gtc/type_ptr.hpp>

struct SubSurface {
    int num_faces;
    int* indeces_ptr; // This points into the side_indeces in MapData bellow
    SubSurface(int f, int* i) : num_faces(f), indeces_ptr(i) {};
};

struct MapData {
    void randomizeCells();
    void establishSides();
    std::vector<SubSurface> interior_surfaces;
    std::vector<SubSurface> adjacent_surfaces;
    bool load_cell[120] = {false};
    std::map<int, PentagonMemory> pentagons;
    bool load_side[120*12] = {false};
private:
    void resetStructure();
    std::vector<int> side_indeces; // content range: 0-120*12
    int side_count, num_faces, sub_idx;
    int* face_ptr;    
    
};

struct PlayerContext {
    PlayerContext();
    ~PlayerContext();
    void initializeMapData();
    void populateDodecaplexVAO();
    void drawMainVAO();
    void drawShrapnelVAOs();
    void updateOldPentagon(int map_index);
    void elapseShrapnel(float progress);
    void elapseGrowth(float progress);
    glm::mat4 getModelMatrix(std::array<bool, 4> WASD, float mouseX, float mouseY, float dt);
    void spawnShrapnel(int map_index);
    PlayerLocation* player_location = NULL;
    MapData map_data;
private:
    template<int N>
    std::array<int, N> getTargetedSurfaces();
    glm::mat4 shrapnel_scatter = glm::mat4(1.0f);
    CPUBufferPair dodecaplex_buffers;
    VAO dodecaplex_vao;
    std::vector<VAO> shrapnel_vaos;
    /* RhombusPattern normal_web   = RhombusPattern(WebType::SIMPLE_STAR, false);
    RhombusPattern inverted_web = RhombusPattern(WebType::SIMPLE_STAR, true);
    float starting_texture = 2.0f;
    float flipped_texture  = 1.0f;
    float shrapnel_texture = 2.0f; */

    RhombusPattern normal_web   = RhombusPattern(WebType::DOUBLE_STAR, false);
    RhombusPattern inverted_web = RhombusPattern(WebType::DOUBLE_STAR, true);
    float starting_texture = 4.0f;
    float flipped_texture  = 4.0f;
    float shrapnel_texture = 4.0f;
};


#endif
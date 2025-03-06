#ifndef WORLD_H
#define WORLD_H

#include "bufferObjects.h"
#include "playerLocation.h"

#include <time.h>
#include <vector>
#include <tuple>
#include <set>
#include <glm/gtc/type_ptr.hpp>

struct SubSurface {
    int faces;
    int* indeces;
    SubSurface(int f, int* i) : faces(f), indeces(i) {};
};

struct MapData {
    void establishMap();
    std::vector<SubSurface> interior_surfaces;
    std::vector<SubSurface> adjacent_surfaces;
    bool load_cell[120] = {false};
private:    
    std::vector<int> side_indeces;
    bool load_side[120*12] = {false};
};

struct PlayerContext {
    PlayerContext();
    void initializeWorldData();
    void establishVAOContext();
    void drawAllVAOs();
    
    glm::mat4 getModelMatrix(std::array<bool, 4> WASD, float mouseX, float mouseY, float dt);
    PlayerLocation* player_location = NULL;
    MapData map_data;
private:
    VAO dodecaplex_vao;
    std::tuple<int, int, int> loadPentagon(GLuint* pentagon_indeces, GLfloat* v_buff, GLuint* i_buff, int v_head, int i_head, int offset);
    std::vector<VAO> additional_vaos;
};


#endif
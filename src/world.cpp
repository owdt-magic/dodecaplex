#include "world.h"
#include "debug.h"
#include "glm/gtx/string_cast.hpp"
#include <stdexcept>

using namespace glm;
using namespace std;

#define SIDES 12
#define CELLS 120
#define VERT_ELEM_COUNT 7
#define DEBUG

void MapData::randomizeCells(){
    for (int i = 0; i < CELLS; i++) {
        load_cell[i] = rand()%2;
    }
    load_cell[0] = true; //Origin should load around player
    load_cell[neighbor_side_orders[0]] = false; //Floor should load below player
};
void MapData::resetStructure(){
    fill(begin(load_side), end(load_side), false);
    side_count = 0;
    side_indeces.clear();
    interior_surfaces.clear();
    adjacent_surfaces.clear();
};
void MapData::establishSides(){
    resetStructure();
    bool side_checklist[CELLS*SIDES] = {false};
    // For every side, we determine whether it should be rendered.
    for (int ci = 0; ci < CELLS; ci++) {
        if (!load_cell[ci]) continue;
        for (int oi = ci*SIDES; oi < ci*SIDES + SIDES; oi++){
            if(load_cell[neighbor_side_orders[oi]]) continue;
            load_side[oi] = true;
            side_count++;
        }
    }
    
    // Third, we will conjoin all adjacent faces in 2 ways....
    side_indeces.reserve(side_count*2);

    function<void(int,int,int,int*)> emplaceNeighbors = [&] 
            (int side_idx, int depth, int freq, int* array) {
        // Recursively visits neighbors to concatenate surface indeces
        side_indeces.push_back(side_idx);
        num_faces++;
        side_checklist[side_idx] = false;
        if (depth < 6) {
            for (int f = 0; f < freq; f++){
                sub_idx = array[side_idx*freq+f];
                if (side_checklist[sub_idx]) {
                    emplaceNeighbors(sub_idx, depth++, freq, array);
                }
            }
        }
    };
    auto populateSurfaces = [&] 
            (int freq, int* array, vector<SubSurface>& surfaces) {
        // Loads SubSurface structures
        copy(begin(load_side), end(load_side), begin(side_checklist));    
        for (int si = 0; si < CELLS*SIDES; si++){
            num_faces = 0;
            if (side_checklist[si]) {
                face_ptr = &*side_indeces.end();
                emplaceNeighbors(si, 0, freq, array);
                surfaces.push_back(SubSurface(num_faces, face_ptr));
            }
        }
    };

    // 1 : Interior (shared cell / concave)
    populateSurfaces(5, &interior_side_indeces[0], interior_surfaces);
    // 2 : Adjacent (nearby cells / convex)
    populateSurfaces(10, &adjacent_side_indeces[0], adjacent_surfaces);

    #ifdef DEBUG
    auto coutSizeHist = [] (vector<SubSurface>& surfaces) {
        int counts[SIDES] = {0};
        cout << "\t";
        for (SubSurface s : surfaces) counts[s.num_faces-1]++;        
        for (int c : counts) cout << c << ", ";
        cout << " (size hist)" << endl;
    };
    cout << "(#subsurfaces) " << endl;
    cout << "  Interior : " << interior_surfaces.size() << " (total)" << endl;
    coutSizeHist(interior_surfaces);
    cout << "  Adjacent : " << adjacent_surfaces.size() << " (total)" << endl;
    coutSizeHist(adjacent_surfaces);
    #endif
};

PlayerContext::PlayerContext() {
    player_location = new PlayerLocation();

    srand(time(NULL)); // Randomize the map...
    
    size_t vertex_max_size = 120*12*normal_web.vertex_count*VERT_ELEM_COUNT*sizeof(GLfloat);
    size_t index_max_size  = 120*12*normal_web.index_count*sizeof(GLuint);
    
    dodecaplex_buffers = CPUBufferPair(vertex_max_size, index_max_size);
};
PlayerContext::~PlayerContext() {
    if (player_location)            free(player_location);
    if (dodecaplex_buffers.v_buff)  free(dodecaplex_buffers.v_buff);
    if (dodecaplex_buffers.i_buff)  free(dodecaplex_buffers.i_buff);
};
void PlayerContext::initializeMapData(){
    map_data.randomizeCells();
    map_data.establishSides();
};
void PlayerContext::populateDodecaplexVAO() {
    int* surface_ptr;
    PentagonMemory memory;
    normal_web.web_texture = starting_texture;

    for (SubSurface surface : map_data.interior_surfaces) {
        surface_ptr = surface.indeces_ptr;
        
        for (int f = 0; f < surface.num_faces; f++) {
            
            if (map_data.pentagons.find(*surface_ptr) == map_data.pentagons.end()) {
                memory = PentagonMemory(*surface_ptr);                

                memory.markStart(dodecaplex_buffers);
                normal_web.buildArrays(dodecaplex_buffers, memory);
                memory.markEnd(dodecaplex_buffers);

                map_data.pentagons[*surface_ptr++] = memory;
            }
        }
    }
     
    dodecaplex_vao = VAO(dodecaplex_buffers);
    
    dodecaplex_vao.LinkAttrib(dodecaplex_vao.vbo, 0, 4, GL_FLOAT, VERT_ELEM_COUNT*sizeof(float), (void*)0);
    dodecaplex_vao.LinkAttrib(dodecaplex_vao.vbo, 1, 3, GL_FLOAT, VERT_ELEM_COUNT*sizeof(float), (void*)(4*sizeof(float)));
    
};
void PlayerContext::updateOldPentagon(int map_index) {    
    inverted_web.web_texture = flipped_texture;

    PentagonMemory& pentagon = map_data.pentagons[map_index];
    pentagon.surface = Surface::BROKEN;
    
    dodecaplex_buffers.setHead(pentagon.v_start, pentagon.i_start, pentagon.i_offset);

    inverted_web.buildArrays(dodecaplex_buffers, pentagon);
    if (dodecaplex_buffers.v_head != pentagon.v_end || 
        dodecaplex_buffers.i_head != pentagon.i_end) {
            std::cout << dodecaplex_buffers.i_head << ", "<< pentagon.i_end << std::endl;
        throw std::invalid_argument("Bad buffer writing length.");
    }

    dodecaplex_vao.UpdateAttribSubset(dodecaplex_vao.vbo, pentagon.v_start*sizeof(GLfloat), 
            pentagon.v_len*sizeof(GLfloat), (void*) &dodecaplex_buffers.v_buff[pentagon.v_start]);
    /* dodecaplex_vao.UpdateAttribSubset(dodecaplex_vao.ebo, pentagon.i_start*sizeof(GLuint), 
            pentagon.i_len*sizeof(GLuint),  (void*) &dodecaplex_buffers.i_buff[pentagon.i_start]); */
    // Since this is a flipped array, with 1:1 index patterning, we can skip this step!
};
void PlayerContext::spawnShrapnel(int map_index) {
    static size_t float_count;
    static size_t uint_count;
    
    inverted_web.web_texture = shrapnel_texture;
    normal_web.web_texture   = shrapnel_texture;
    
    PentagonMemory pentagon = map_data.pentagons[map_index];
    float_count = (2*pentagon.v_len*(7+4)/7)*sizeof(GLfloat);
    uint_count  = 2*pentagon.i_len*sizeof(GLuint);
    CPUBufferPair shrapnel_buffer = CPUBufferPair(float_count, uint_count);
    
    normal_web.buildArrays(  shrapnel_buffer, pentagon, true);
    inverted_web.buildArrays(shrapnel_buffer, pentagon, true);

    VAO shrapnel_vao = VAO(shrapnel_buffer);
    
    shrapnel_vao.LinkAttrib(shrapnel_vao.vbo, 0, 4, GL_FLOAT, (VERT_ELEM_COUNT+4)*sizeof(float), (void*)0);
    shrapnel_vao.LinkAttrib(shrapnel_vao.vbo, 1, 3, GL_FLOAT, (VERT_ELEM_COUNT+4)*sizeof(float), (void*)(4*sizeof(float)));    
    shrapnel_vao.LinkAttrib(shrapnel_vao.vbo, 2, 4, GL_FLOAT, (VERT_ELEM_COUNT+4)*sizeof(float), (void*)(7*sizeof(float)));

    shrapnel_vaos.push_back(shrapnel_vao);
    
    free(shrapnel_buffer.i_buff);
    free(shrapnel_buffer.v_buff);
};
int PlayerContext::getTargetedSurface(){
    vec4 center;
    mat4 transform = player_location->currentTransform();
    int best_index = -1;
    int surface_index;
    const float max_cell_dist = -0.1f;
    float best_distance = -1000.0f;
    float projected_dist;

    for (int i = 0; i < CELLS; i++) {
        if (map_data.load_cell[i]) {
            center = transform*dodecaplex_centroids[i];
            if (center.z < max_cell_dist) {
                for (int j = 0; j < SIDES; j++){
                    surface_index = i*SIDES+j;
                    if (map_data.load_side[surface_index]) {
                        center = transform*map_data.pentagons[surface_index].offset;
                        projected_dist = center.z*ROOT_FIVE/(ROOT_FIVE+center.w);
                        if ((center.x*center.x+center.y*center.y < 0.2f) &&
                            (projected_dist > best_distance ) && (projected_dist < 0.0f)){
                            best_index = surface_index;
                            best_distance = projected_dist;
                        }
                    }
                }
            }
        }
    }
    return best_index;
};
void PlayerContext::elapseShrapnel(float progress) {
    int target_index;
    if ( progress < 0.2) { shrapnel_vaos.clear(); return; }   //TODO: HIGH ERROR POTENTIAL!!
    target_index = getTargetedSurface();
    if ((target_index > -1) && (map_data.pentagons[target_index].surface != Surface::BROKEN)) {
        spawnShrapnel(target_index);
        updateOldPentagon(target_index);
    }
};
void PlayerContext::elapseGrowth(float progress){
    int target_index = getTargetedSurface();
    if (target_index < 0) return;
    map_data.load_cell[neighbor_side_orders[target_index]] = true;
    map_data.establishSides();
    populateDodecaplexVAO();
};
void PlayerContext::drawMainVAO(){
    dodecaplex_vao.DrawElements(GL_TRIANGLES);
};
void PlayerContext::drawShrapnelVAOs(){
    for (int i = 0; i < shrapnel_vaos.size(); i++){     
        shrapnel_vaos[i].DrawElements(GL_TRIANGLES);
    }
};
mat4 PlayerContext::getModelMatrix(array<bool, 4> WASD, float mouseX, float mouseY, float dt) {
    if (!player_location->overridden) {
        player_location->focusFromMouse(mouseX, mouseY, dt);
        player_location->positionFromKeys(WASD, dt);  

        return player_location->getModel(&map_data.load_cell[0]);
    } else {
        return player_location->elapseAnimation(dt);
    }
}

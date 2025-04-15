#include "world.h"
#include "debug.h"
#include "glm/gtx/string_cast.hpp"
#include <stdexcept>
#include <algorithm>

using namespace glm;
using namespace std;

#define SIDES 12
#define CELLS 120
#define VERT_ELEM_COUNT 7
#define DEBUG

void MapData::randomizeCells(){
    for (int i = 0; i < CELLS; i++) {
        load_cell[i] = rand()%3;
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
    interior_surfaces.clear();
    adjacent_surfaces.clear();
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
void MapData::buildPentagonMemory(){
    int index;
    for (int i=0; i < CELLS*SIDES; ++i){
        if ((!load_side[i]) || (pentagons.find(i) != pentagons.end())) continue;
        pentagons[i] = PentagonMemory(i);
    }
    //TODO: building the side adjacently info could be optimized, to reduce redundancy...
    for (auto& pair : pentagons){
        for (int i=0; i<5; ++i){
            index = interior_side_indeces[pair.first*5+i];
            if ((pentagons.find(index) != pentagons.end()) && (index != pair.first)){
                pair.second.addNeighbor(&(pentagons[index]), false);
            }
        }
        for (int i=0; i<10; ++i){
            index = adjacent_side_indeces[pair.first*10+i];
            if ((pentagons.find(index) != pentagons.end()) && (index != pair.first)){
                pair.second.addNeighbor(&(pentagons[index]), true);
            }
        }
    }
}

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
    map_data.buildPentagonMemory();
};
void PlayerContext::populateDodecaplexVAO() {
    int* surface_ptr;
    normal_web.web_texture = starting_texture;
    dodecaplex_buffers.reset();

    for (SubSurface surface : map_data.adjacent_surfaces) {
        surface_ptr = surface.indeces_ptr;
        for (int f = 0; f < surface.num_faces; f++) {
            PentagonMemory& memory = map_data.pentagons[*surface_ptr++];
            memory.markStart(dodecaplex_buffers);
            normal_web.buildArrays(dodecaplex_buffers, memory);
            memory.markEnd(dodecaplex_buffers);
        }
    }
     
    dodecaplex_vao = VAO(dodecaplex_buffers);
    dodecaplex_vao.LinkVecs({4,3}, 7);    
};
void PlayerContext::damageOldPentagon(int map_index) {    
    PentagonMemory& pentagon = map_data.pentagons[map_index];
    PentagonMemory* other;
    normal_web.applyDamage(dodecaplex_buffers, player_location->currentTransform(), pentagon);

    dodecaplex_vao.UpdateAttribSubset(dodecaplex_vao.vbo, pentagon.v_start*sizeof(GLfloat), 
            pentagon.v_len*sizeof(GLfloat), (void*) &dodecaplex_buffers.v_buff[pentagon.v_start]);

    for (int i=0; i<5; ++i) {
        other = pentagon.neighbors[i].first;
        normal_web.applyDamage(dodecaplex_buffers, player_location->currentTransform(), *other);

        dodecaplex_vao.UpdateAttribSubset(dodecaplex_vao.vbo, other->v_start*sizeof(GLfloat), 
                other->v_len*sizeof(GLfloat), (void*) &dodecaplex_buffers.v_buff[other->v_start]);        
    }
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
    
    shrapnel_vao.LinkVecs({4,3,4}, 11);

    shrapnel_vaos.push_back(shrapnel_vao);
    
    free(shrapnel_buffer.i_buff);
    free(shrapnel_buffer.v_buff);
};
template<int N>
array<int, N> PlayerContext::getTargetedSurfaces(){
    mat4 transform = player_location->currentTransform();
    vec4 center;
    float best_center_dist   = -1000.0f,
          projected_dist;    
    
    array<int, N> output = {-1};
    vector<pair<int, float>> metrics;
    
    auto projectPoint = [](vec4 in) {
        return in.z*ROOT_FIVE/(ROOT_FIVE+in.w);
    };

    for (int i = 0; i < CELLS; i++) {
        if (!map_data.load_cell[i])
            continue;
        center = transform*dodecaplex_centroids[i];
        if (projectPoint(center) > 0.1f)
            continue;
        for (int j = i*SIDES; j < (i+1)*SIDES; j++){
            if (!map_data.load_side[j])
                continue;
            center = transform*map_data.pentagons[j].offset;
            projected_dist = projectPoint(center);
            if ((center.x*center.x+center.y*center.y > 0.4f) || (projected_dist > 0.0f)) 
                continue;
            metrics.push_back(make_pair(j, projected_dist));
            best_center_dist = std::max(best_center_dist, projected_dist);
        }
    }
    sort(metrics.begin(), metrics.end(), [](pair<int,float> a, pair<int,float> b){
        return a.second > b.second;
    });
    for (int i = 0; i < std::min(N, (int) metrics.size()); ++i){
        if (metrics[i].second >= best_center_dist-0.4f) {
            output[i] = metrics[i].first;
        }        
    }
    return output;
};
void PlayerContext::elapseShrapnel(float progress) {
    for (int target_index : getTargetedSurfaces<3>()) {
        if (target_index < 0) break;
        if (progress == 1.0f) spawnShrapnel(target_index);
        damageOldPentagon(target_index);
    }
    if ( progress == 0.0f) shrapnel_vaos.clear();
};
void PlayerContext::elapseGrowth(float progress){
    if ( progress == 1.0f) {
        for (int target_index : getTargetedSurfaces<1>()){
            if (target_index < 0) return;
            map_data.load_cell[target_index/12] = false;
            map_data.establishSides();
            map_data.buildPentagonMemory();
            populateDodecaplexVAO();
        }
    }
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

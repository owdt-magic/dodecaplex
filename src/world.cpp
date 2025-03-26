#include "world.h"
#include "triangularization.h"
#include "debug.h"
#include "glm/gtx/string_cast.hpp"
#include <stdexcept>

using namespace glm;
using namespace std;

#define SIDES 12
#define CELLS 120
#define VERT_ELEM_COUNT 7
#define VERT_PER_PENT 11
#define TRI_PER_PENT 15
#define DEBUG

void MapData::establishMap(){
    int side_count = 0,
        num_faces, sub_idx;
    int* face_ptr;    
    bool side_checklist[CELLS*SIDES];

    // First, for all 120 cells we set whether they are loaded, 
    //  aka can the player enter them?
    for (int i = 0; i < CELLS; i++) {
        load_cell[i] = rand()%2;
    }
    load_cell[0] = true; //Origin should load around player
    load_cell[neighbor_side_orders[0]] = false; //Floor should load below player

    // Second, for every side, we determine whether it should be rendered.
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
    
    vertex_max_size = 120*12*VERT_PER_PENT*VERT_ELEM_COUNT*sizeof(GLfloat);
    index_max_size  = 120*12*TRI_PER_PENT*3*sizeof(GLuint);
    
    vertex_buffer = (GLfloat*) malloc(vertex_max_size);
    index_buffer  = (GLuint*)  malloc(index_max_size);

    if ((vertex_buffer == NULL) || (index_buffer == NULL)) {
        throw std::runtime_error("Failed to initialize buffers for dodecaplex with sizes: "+
            std::to_string(vertex_max_size)+" and "+ std::to_string(index_max_size));
    }
};
PlayerContext::~PlayerContext() {
    free(player_location);
    free(index_buffer);
    free(vertex_buffer);
};
void PlayerContext::initializeMapData(){
    map_data.establishMap();
};

array<vec4, 5> readDestinationCoords(GLuint* pentagon_ptr){
    array<vec4, 5> output;
    for (int i = 0; i < 5; i++) {
        output[i] = vec4(
            dodecaplex_cell_verts[pentagon_ptr[i]*4],
            dodecaplex_cell_verts[pentagon_ptr[i]*4+1],
            dodecaplex_cell_verts[pentagon_ptr[i]*4+2],
            dodecaplex_cell_verts[pentagon_ptr[i]*4+3]
        );
    }
    return output;
};

pair< array<vec4, 5>, array<vec4, 2> > unpackPentagon(int side_idx) {
    GLuint* pentagon_ptr    = &dodecaplex_penta_indxs[side_idx*5];
    GLfloat* inner_cent_ptr = &dodecaplex_centroids[(side_idx/12)*4];
    GLfloat* outer_cent_ptr = &dodecaplex_centroids[neighbor_side_orders[side_idx]*4];
    
    array<vec4, 5> dest_corners = readDestinationCoords(pentagon_ptr);
    array<vec4, 2> dest_centroids = {
        vec4(inner_cent_ptr[0], inner_cent_ptr[1], inner_cent_ptr[2], inner_cent_ptr[3]),
        vec4(outer_cent_ptr[0], outer_cent_ptr[1], outer_cent_ptr[2], outer_cent_ptr[3])
    };
    return std::make_pair(dest_corners, dest_centroids);
};

PentagonMemory loadNewPentagon(int* index_ptr, GLfloat* v_buff, GLuint* i_buff, int& v_head, int& i_head, uint& offset) {        
    static RhombusWeb simple_web = RhombusWeb(WebType::SIMPLE_STAR, true);

    pair< array<vec4, 5>, array<vec4, 2> > dest_vecs = unpackPentagon(*index_ptr);
    
    simple_web.buildArrays(v_buff, i_buff, v_head, i_head, offset, dest_vecs.first, dest_vecs.second);
    
    offset += simple_web.offset;
    
    return PentagonMemory(v_head, i_head, simple_web.offset, *index_ptr);
};

void PlayerContext::populateDodecaplexVAO() {
    int v_head = 0, i_head = 0;
    uint offset = 0;
    int* surface_ptr;
    PentagonMemory memory;

    for (SubSurface surface : map_data.interior_surfaces) {
        surface_ptr = surface.indeces_ptr;
        for (int f = 0; f < surface.num_faces; f++) {
            memory = loadNewPentagon( surface_ptr, vertex_buffer, index_buffer, v_head, i_head, offset);
            map_data.pentagon_summary[*surface_ptr++] = memory;
        }
    }
     
    dodecaplex_vao = VAO((GLfloat*) vertex_buffer, v_head*sizeof(GLfloat), (GLuint*) index_buffer, i_head*sizeof(GLfloat));

    dodecaplex_vao.LinkAttrib(dodecaplex_vao.vbo, 0, 4, GL_FLOAT, VERT_ELEM_COUNT*sizeof(float), (void*)0);
    dodecaplex_vao.LinkAttrib(dodecaplex_vao.vbo, 1, 3, GL_FLOAT, VERT_ELEM_COUNT*sizeof(float), (void*)(4*sizeof(float)));
};

void updateOldPentagon(PentagonMemory memory) {
    pair< array<vec4, 5>, array<vec4, 2> > dest_vecs = unpackPentagon(memory.source);
        
    //simple_web.buildArrays(v_buff, i_buff, v_head, i_head, offset, dest_vecs.fist, dest_vecs.second);
    
};

void PlayerContext::drawAllVAOs() {
    dodecaplex_vao.DrawElements(GL_TRIANGLES);
    for (int i = 0; i < additional_vaos.size(); i++){
        additional_vaos[i].DrawElements(GL_TRIANGLES);
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

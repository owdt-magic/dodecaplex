#include "world.h"
#include "triangularization.h"
#include "debug.h"
#include "glm/gtx/string_cast.hpp"

using namespace glm;
using namespace std;

#define SIDES 12
#define CELLS 120
#define VERT_ELEM_COUNT 7
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
    if (player_location == NULL) {
        player_location = new PlayerLocation();
    }
    srand(time(NULL)); // Randomize the map...
};
void PlayerContext::initializeWorldData(){
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

void loadPentagon(int* index_ptr, GLfloat* v_buff, GLuint* i_buff, int& v_head, int& i_head, uint& offset) {        
    GLuint* pentagon_ptr    = &dodecaplex_penta_indxs[*index_ptr*5];
    GLfloat* inner_cent_ptr = &dodecaplex_centroids[(*index_ptr/12)*4];
    GLfloat* outer_cent_ptr = &dodecaplex_centroids[neighbor_side_orders[*index_ptr]*4];
    static RhombusWeb simple_web = RhombusWeb();
    
    array<vec4, 5> dest_corners = readDestinationCoords(pentagon_ptr);
    array<vec4, 2> dest_centroids = {
        vec4(*inner_cent_ptr++, *inner_cent_ptr++, *inner_cent_ptr++, *inner_cent_ptr++),
        vec4(*outer_cent_ptr++, *outer_cent_ptr++, *outer_cent_ptr++, *outer_cent_ptr++)
    };
    
    simple_web.buildArrays(v_buff, i_buff, v_head, i_head, offset, dest_corners, dest_centroids);
    
    offset += simple_web.offset;
};

void PlayerContext::establishVAOContext() {
    int v_head = 0, i_head = 0;
    uint offset = 0;
    int* surface_ptr;

    const size_t vertex_max_size = 120*12*11*VERT_ELEM_COUNT*sizeof(GLfloat);
    const size_t index_max_size  = 120*12*11*3*sizeof(GLuint);
    
    GLfloat* vertex_buffer = (GLfloat*) malloc(vertex_max_size);
    GLuint* index_buffer   = (GLuint*) malloc(index_max_size);

    if ((vertex_buffer == NULL) || (index_buffer == NULL)) {
        
    }
    int count = 0;
    for (SubSurface surface : map_data.interior_surfaces) {
        surface_ptr = surface.indeces_ptr;
        for (int f = 0; f < surface.num_faces; f++) {
            loadPentagon( surface_ptr++, vertex_buffer, index_buffer, v_head, i_head, offset);
        }
        count++;
        if (count > 15) break;
    }
     
    dodecaplex_vao = VAO((GLfloat*) vertex_buffer, v_head*sizeof(GLfloat), (GLuint*) index_buffer, i_head*sizeof(GLfloat));

    dodecaplex_vao.LinkAttrib(dodecaplex_vao.vbo, 0, 4, GL_FLOAT, VERT_ELEM_COUNT*sizeof(float), (void*)0);
    dodecaplex_vao.LinkAttrib(dodecaplex_vao.vbo, 1, 3, GL_FLOAT, VERT_ELEM_COUNT*sizeof(float), (void*)(4*sizeof(float)));

    free(index_buffer);
    free(vertex_buffer);

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

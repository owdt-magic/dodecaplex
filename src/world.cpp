#include "world.h"
#include "debug.h"

using namespace glm;
using namespace std;

void MapData::establishMap(){
    int side_count = 0;
    int num_faces, sub_idx;
    int* face_ptr;    
    bool side_checklist[120*12];

    // First, for all 120 cells we set whether they are loaded, 
    //  aka can the player enter them?
    for (int i = 0; i < 120; i++) {
        load_cell[i] = rand()%2;
    }
    load_cell[0] = true;
    load_cell[neighbor_side_orders[0]] = false;

    // Second, for every side, we determine whether it should be rendered.
    for (int ci = 0; ci < 120; ci++) {
        if (load_cell[ci]) {
            for (int ord_idx = ci*12; ord_idx < ci*12 + 12; ord_idx++){
                if (!load_cell[neighbor_side_orders[ord_idx]]) {
                    load_side[ord_idx] = true;
                    side_count++;
                }
            }
        }
    }
    
    function<void(int,int,int,int*)> emplaceNeighbors = [&] (int side_idx, int depth, int freq, int* array) {
        side_indeces.push_back(side_idx);
        num_faces++;
        side_checklist[side_idx] = false;
        if (depth < 3) {
            for (int f = 0; f < freq; f++){
                sub_idx = array[side_idx*freq+f];
                if (side_checklist[sub_idx]) {
                    emplaceNeighbors(sub_idx, depth++, freq, array);
                }
            }
        }
    };
            
    side_indeces.reserve(side_count*2);

    copy(begin(load_side), end(load_side), begin(side_checklist));    
    for (int si = 0; si < 120*12; si++){
        num_faces = 0;
        if (side_checklist[si]) {
            face_ptr = &*side_indeces.end();
            emplaceNeighbors(si, 0, 5, &interior_side_indeces[0]);
            interior_surfaces.push_back(SubSurface(num_faces, face_ptr));
        }
    }

    copy(begin(load_side), end(load_side), begin(side_checklist));    
    for (int si = 0; si < 120*12; si++){
        num_faces = 0;
        if (side_checklist[si]) {
            face_ptr = &*side_indeces.end();
            emplaceNeighbors(si, 0, 10, &adjacent_side_indeces[0]);
            adjacent_surfaces.push_back(SubSurface(num_faces, face_ptr));
        }
    }

    std::cout << side_indeces.size() << ", " << side_count << ", " <<  
        interior_surfaces.size() << ", " << adjacent_surfaces.size() << std::endl;
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

#define VERT_ELEM_COUNT 7

tuple<int, int, int> PlayerContext::loadPentagon(GLuint* pentagon_indeces, GLfloat* v_buff, GLuint* i_buff, int v_head, int i_head, int offset) {
    /* pentagon_indeces : address of first of the 4 pentagon indeces defined in dodecaplex.h
        offset : number of vertex elements written to be drawn so far. */
    const GLfloat texture_pattern[] = {0.48f,0.0f,  0.0f,0.38f,  0.2f,0.95f,  0.8f,0.95f,  1.0f,0.38f};
    const GLuint index_pattern[] = {0,1,2,  0,2,3,  0,3,4};
    const int num_verts = 5, num_dims = 4;
    float texture_img_idx = 2.0f;
    int t = 0;
    GLfloat mean_vals[num_dims] = {0.0f};

    for (int v = 0; v < num_verts; v++){
        for (int d = 0; d < num_dims; d++){
            mean_vals[d] += dodecaplex_cell_verts[pentagon_indeces[v]*4+d];
        }
    }
    for (int v = 0; v < num_verts; v++){
        for (int d = 0; d < num_dims; d++){
            v_buff[v_head++] = dodecaplex_cell_verts[pentagon_indeces[v]*4+d]*1.0f + mean_vals[d]*0.00;
        }
        v_buff[v_head++] = texture_pattern[t++];
        v_buff[v_head++] = texture_pattern[t++];
        v_buff[v_head++] = texture_img_idx;
    }
    
    for (GLuint idx : index_pattern) {
        i_buff[i_head++] = offset+idx;
    }

    offset += num_verts;
    return {v_head, i_head, offset};
};

void PlayerContext::establishVAOContext() {
    int v_head = 0, i_head = 0, offset = 0;

    const size_t vertex_max_size = 120*12*5*VERT_ELEM_COUNT*sizeof(GLfloat);
    const size_t index_max_size  = 120*12*5*3*sizeof(GLuint);
    
    GLfloat* vertex_buffer = (GLfloat*) malloc(vertex_max_size);
    GLuint* index_buffer   = (GLuint*) malloc(index_max_size);

    for (SubSurface surface : map_data.interior_surfaces) {
        for (int f = 0; f < surface.faces; f++) {
            tie(v_head, i_head, offset) = loadPentagon( &dodecaplex_penta_indxs[*(surface.indeces++)*5], 
                                                        vertex_buffer, index_buffer, v_head, i_head, offset);
        }
    }
     
    dodecaplex_vao = VAO((GLfloat*) vertex_buffer, v_head*sizeof(GLfloat), (GLuint*) index_buffer, i_head*sizeof(GLfloat));

    dodecaplex_vao.LinkAttrib(dodecaplex_vao.vbo, 0, 4, GL_FLOAT, VERT_ELEM_COUNT*sizeof(float), (void*)0);
    dodecaplex_vao.LinkAttrib(dodecaplex_vao.vbo, 1, 3, GL_FLOAT, VERT_ELEM_COUNT*sizeof(float), (void*)(4*sizeof(float)));

    free(index_buffer);

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
#include "world.h"
#include "debug.h"

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
        for (SubSurface s : surfaces) counts[s.faces-1]++;        
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

void PlayerContext::loadPentagon(GLuint* pentagon_indeces, GLfloat* v_buff, GLuint* i_buff, int& v_head, int& i_head, int& offset) {
    // pentagon_indeces : address of first of the 5 4D pentagon indeces defined in dodecaplex.h
    //    offset : number of vertex elements written to be drawn so far in the buffers
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
};

void PlayerContext::establishVAOContext() {
    int v_head = 0, i_head = 0, offset = 0;

    const size_t vertex_max_size = 120*12*5*VERT_ELEM_COUNT*sizeof(GLfloat);
    const size_t index_max_size  = 120*12*5*3*sizeof(GLuint);
    
    GLfloat* vertex_buffer = (GLfloat*) malloc(vertex_max_size);
    GLuint* index_buffer   = (GLuint*) malloc(index_max_size);

    if ((vertex_buffer == NULL) || (index_buffer == NULL)) {
        
    }

    for (SubSurface surface : map_data.interior_surfaces) {
        for (int f = 0; f < surface.faces; f++) {
            loadPentagon( &dodecaplex_penta_indxs[*(surface.indeces++)*5], 
                          vertex_buffer, index_buffer, v_head, i_head, offset);
        }
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

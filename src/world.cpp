#include "world.h"

#define MAX_CELL_VERTS 60 
    // 20 corners x3 redundancy for unique sides. 
    // (3 meet per corner with 3 unique textures)
#define MAX_CELL_FACES 12*3
    // need 3 triangles for a single pentagon.

using namespace glm;

PlayerContext::PlayerContext() {
    if (player_location == NULL) {
        player_location = new PlayerLocation();
    }
    srand(time(NULL)); // Randomize the map...
};
VAO cellToVAO(WorldCell& cell, GLfloat* cell_vert_buff, GLuint* cell_indx_buff,\
                             std::size_t v_buff_size, std::size_t i_buff_size) {
    
    std::size_t verts_size = cell.generateVerts(cell_vert_buff, v_buff_size);
    std::size_t indxs_size = cell.generateIndxs(cell_indx_buff, i_buff_size);
    
    VAO cell_vao(
        (GLfloat*) cell_vert_buff, verts_size,
        (GLfloat*) &cell.cell_matrix, sizeof(mat4),
        (GLuint*) cell_indx_buff, indxs_size
    );

    cell_vao.LinkAttrib(cell_vao.vbo, 0, 3, GL_FLOAT, VERTEX_ELEMENT_COUNT * sizeof(float), (void*)0); // xyz
    cell_vao.LinkAttrib(cell_vao.vbo, 1, 3, GL_FLOAT, VERTEX_ELEMENT_COUNT * sizeof(float), (void*)(3*sizeof(float))); // uv i
    cell_vao.LinkMat4(cell_vao.cbo, 2);
    return cell_vao;
};

std::vector<WorldCell*> PlayerContext::establishNeighborhood() {    
    std::vector<WorldCell*> neighbors = {player_location->reference_cell};
    static int n, generation_size, last_generation = 0;
    static const int 
        max_adjacent = 5,
        max_attempts = 100,
        max_generations = 3;

    WorldCell* new_cell;

    auto notTooClose = [&](WorldCell* cell) {
        for (WorldCell* other_cell : neighbors)
            if (!cell->notTooClose(other_cell)) return false;
        return true;
    };
    auto assignNeighbors = [&](WorldCell* cell) {
        for (int i = 0; i < max_adjacent; i++) {
            for (int j = 0; j < max_attempts; j++) {
                n = rand()%12;
                if (!cell->canAddDoor(n)) continue;
                    //^ Avoids unnecessary cell initialization...
                new_cell = new WorldCell(cell, n);
                if (notTooClose(new_cell)) {
                    cell->addDoor(n, *new_cell);
                    neighbors.push_back(new_cell);
                    break;
                } else {free(new_cell);}
            }
        }
    };

    for (int i = 0; i < max_generations; i++) {
        generation_size = neighbors.size();
        for (int j = last_generation; j < generation_size; j++) {
            assignNeighbors(neighbors[j]);
        }
        last_generation = generation_size;
    }
    return neighbors;
};

std::size_t initializeDodecaplexStates(GLuint* index_buffer){
    int dest = 0,
        read = 0,
        n,m,o,p;
    bool load_cell[120] = {false};
    
    for (int i =0; i < 12; i++) {
        n = neighbor_side_orders[i];
        load_cell[n] = true;//!(rand()%2);
        for (int j=0; j < 12; j++) {
            m = neighbor_side_orders[n*12 + j];
            if (!load_cell[m]){
                load_cell[m] = !(rand()%5);
            } //Only 2 levels of recursion until fixing 'wrapping'
            /* for (int k=0; k < 12; k++) {
                o = neighbor_side_orders[m*12 + k];
                if (!load_cell[o]){
                    load_cell[o] = !(rand()%20);
                }                
            } */
        }
    }
    load_cell[0] = true;

    // Using load_cell array, we condtionally draw each pentagon in the dodecaplex
    for (int ci = 0; ci < 120; ci++) {
        if (load_cell[ci]) {
            for (int ord_idx = ci*12; ord_idx < ci*12 + 12; ord_idx++){
                if (!load_cell[neighbor_side_orders[ord_idx]]) {
                    // Current cell is being drawn, neighbor is not...
                    // the requires we draw the wall / face...
                    for (int nine = 0; nine < 9; nine++) {
                        // 3 triangles / face * 3 points / triangle
                        index_buffer[dest++] = dodecaplex_cell_indxs[read++];
                    }
                } else read += 9;
            }
        } else read += 108;
    }

    return (std::size_t) sizeof(GLuint)*dest;
};

void PlayerContext::linkDodecaplexVAOs() {
    std::size_t index_max_size = 120*36*3*sizeof(GLuint);
    
    GLuint* index_buffer = (GLuint*) malloc(index_max_size);
    
    std::size_t index_real_size = initializeDodecaplexStates(index_buffer);

    VAO dodecaplex_vao(
        (GLfloat*) &dodecaplex_cell_verts, sizeof(GLfloat)*600*4,
        (GLuint*) index_buffer, index_real_size
    );

    dodecaplex_vao.LinkAttrib(dodecaplex_vao.vbo, 0, 4, GL_FLOAT, 4*sizeof(float), (void*)0);

    all_vaos.push_back(dodecaplex_vao);

    free(index_buffer);

};

void PlayerContext::drawAllVAOs() {
    for (int i = 0; i < all_vaos.size(); i++){
        all_vaos[i].DrawElements(GL_TRIANGLES);
    }    
};

void PlayerContext::linkPlayerCellVAOs() {
    std::vector<WorldCell*> neighborhood = establishNeighborhood();

    std::size_t v_size = MAX_CELL_VERTS*6*sizeof(GLfloat);
    std::size_t i_size = MAX_CELL_FACES*3*sizeof(GLuint);

    GLfloat* v_buff = (GLfloat*) malloc(v_size);
    GLuint* i_buff  = (GLuint*) malloc(i_size);

    for (int i = 0; i < neighborhood.size(); i++) {
        all_vaos.push_back(cellToVAO(*neighborhood[i], v_buff, i_buff, v_size, i_size));
    }

    free(v_buff);
    free(i_buff);
};    

PlayerLocation::PlayerLocation() {
    if (reference_cell == NULL) reference_cell = new WorldCell();
    floor_indx  = getFloorIndex();
    
    #ifdef LEGACY
    player_up   = reference_cell->floor_norms[floor_indx];
    vec3 floor  = reference_cell->sides[floor_indx].findIntercept(vec3(0.),-player_up);
    head = floor + player_up*height;
    #endif    
};
mat4 PlayerLocation::getView(float x, float y, float dt) {
    updateFocus(x, y, dt); // Account for mouse movement...
    mat4 View = lookAt(
        vec3(0), focus, player_up
    );// since this is view, keep the origin at 0, and not head...
    return View;
};
vec3 PlayerLocation::getFocus() {
    return focus;
};
vec3 PlayerLocation::getHead() {
    return head;
};
vec3 PlayerLocation::getPUp() {
    return player_up;
};
float PlayerLocation::getHeight() {
    return height;
};
void PlayerLocation::teleportHead(vec3 target) {
    head = target;
};
void PlayerLocation::teleportPUp(vec3 target) {
    player_up = target;
};

InterceptResult PlayerLocation::getIntercept() {
    vec3 intercept, detransformed_head, detransformed_focus;
    int exit_index = -1;
    CellSide side;
    WorldCell* next_cell = reference_cell;
    
    auto findExitSide = [&](WorldCell* cell) {
        detransformed_head = vec3(inverse(cell->cell_matrix)*vec4(head, 1));
        detransformed_focus = vec3(inverse(cell->reflection_mat)*vec4(focus, 1));
            // detransform is needed as CellSide world co-ordinates are relative
        for ( int i = 0; i < 12; i++) {
            if ( exit_index != i ) { // Since all cells are mirrored, skip the last exit index
                side = cell->sides[i];
                intercept = side.findIntercept(detransformed_head, detransformed_focus);
                if ( intercept != detransformed_head ) {
                    exit_index = i;
                    intercept = cell->origin+vec3(cell->reflection_mat*vec4(intercept,1));
                        // This makes the intercept useful in the actual game.
                    break;
                }
            }
        }
    };
    
    int depth = 0;
    findExitSide(reference_cell);
    while (next_cell->hasDoor(exit_index)) {
        next_cell = next_cell->doors[exit_index];
        findExitSide(next_cell);
        depth++;
        if (depth > 100) throw std::runtime_error("Max door intercept check passed");
    }
    InterceptResult result;
    result.cell = next_cell;
    result.point = intercept;
    result.index = exit_index;
    return result;
}
bool PlayerLocation::accountBoundary(vec3& direction) {
    // This method does two things:
    // makes sure that `player_up` is right based on location
    // returns false if the player is about to hit a wall.
    const float SCALE_MULT = 1/7.0f;
    const float min_dist = length(head - direction - reference_cell->origin);
    
    int adjacent_indx;
    float cur_dist, ref_weight;
    vec3 hypothetical_origin, ref_up, door_up;
    for (int i = 0; i < 5; i++) { // There are 5 adjacent sides...
        adjacent_indx       = Dodecahedron::adjacency_matrix[floor_indx*5 + i]; 
            // Map from 1-5 to cell normal indeces
        hypothetical_origin = reference_cell->origin-reference_cell->floor_norms[adjacent_indx]*NORMAL_SCALE;
        cur_dist            = length(head - direction - hypothetical_origin);
        if ((cur_dist - min_dist) <= NORMAL_SCALE*SCALE_MULT) {
            // Within `SCALE_MULT`% of the edge, you begin to blend the normals together.
            if (reference_cell->doors[adjacent_indx] != NULL) { // Checking : is there a cell through this wall?
                if (cur_dist < min_dist) { 
                    // This is where we cross the cell boundary..
                    if (reference_cell->doors[adjacent_indx]->hasDoor(floor_indx)) {
                        // If there is a door in the floor, don't walk on it!
                        return false;
                    }
                    reference_cell = reference_cell->doors[adjacent_indx];
                    // Otherwise, swap the reference cell once you cross the boundary.
                } 
                ref_up      = reference_cell->floor_norms[floor_indx];
                door_up     = reference_cell->doors[adjacent_indx]->floor_norms[floor_indx];
                ref_weight  = ((cur_dist-min_dist)/(NORMAL_SCALE*SCALE_MULT*2.0f)+0.5f);
                player_up   = normalize(ref_up*ref_weight + door_up* (1.0f-ref_weight));
                    // This above blends the two normals together based on proximity.
                return true; 
                    // There is a door here, the player can pass.
            }
            return false;
        }
    }
    return true;
};
#define NOCLIP
void PlayerLocation::moveHead(std::array<bool, 4> WASD, float dt) {
    vec3 left_right = normalize(cross(focus, -player_up));
    
    #ifndef NOCLIP
    vec3 front_back = normalize(cross(left_right, -player_up));
    #endif    
    #ifdef NOCLIP
    vec3 front_back = normalize(-focus);    
    #endif

    vec3 direction = vec3(0., 0., 0.);
    if (WASD[0]) direction += front_back; //already normalized
    if (WASD[1]) direction -= left_right; normalize(direction);
    if (WASD[2]) direction -= front_back; normalize(direction);
    if (WASD[3]) direction += left_right; normalize(direction);
    direction *= movement_scale*dt;

    #ifndef NOCLIP
    if ( !accountBoundary(direction) ) {
        head -= direction;
    }
    #endif
    #ifdef NOCLIP
    head -= direction;
    #endif
};

glm::mat4 rotationMatrix(glm::vec3 axis){    
    return glm::mat4(
        2.0f*axis.x*axis.x-1.0f, 2.0f*axis.y*axis.x,      2.0f*axis.z*axis.x,      0.0f,
        2.0f*axis.x*axis.y,      2.0f*axis.y*axis.y-1.0f, 2.0f*axis.z*axis.y,      0.0f,
        2.0f*axis.x*axis.z,      2.0f*axis.y*axis.z,      2.0f*axis.z*axis.z-1.0f, 0.0f,
        0.0f,                    0.0f,                    0.0f,                    1.0f
    );
}

glm::mat4 getIntermediateTransformation(glm::vec3 axis){
    // Uses a pre-computed least squares matrix "conversion_matrix"
    // and the direction of "axis" to determine a 4D transformaiton
    // matrix to apply to the verticies of the dodecaplex.
    const float fourth_dim = PHI/2.0f;
    glm::vec4 compass = glm::vec4(1.0f);
    float ratio = glm::length(axis)/(0.25 + 0.25*PHI*PHI);

    axis = normalize(axis);
    compass = compass*rotationMatrix(axis);
    axis = axis*sqrt(1.0f-(fourth_dim*fourth_dim)); 
        // Scale the normalized axis to be normalized with a hypothetical w of fourth_dim
    float charcterization[9] = {compass.x, compass.y, compass.z, compass.w, 
        axis.x*axis.z, axis.x*fourth_dim, axis.y*axis.z, axis.y*fourth_dim, axis.z*fourth_dim};
        // The parameters needed to interpolate the right transformation matrix.
    float weights[4*4] = {0.0f};
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 16; j++) {
            weights[j] += conversion_matrix[i*16+j]*charcterization[i];
        }
    }
    return glm::make_mat4(weights)*ratio + glm::mat4(1.0f)*(1.0f-ratio);
};

mat4 PlayerLocation::getModel(std::array<bool, 4> WASD, float dt) {
    moveHead(WASD, dt);
    return getIntermediateTransformation(head);
};
uint PlayerLocation::getFloorIndex() {
    //Using the players feet this gives the closest aligned
    //surface normal in the dodecahedron defining the cell
    //feet is the displacement from the head
    float min = 0.0;
    int min_idx = 0;
    for (unsigned i; i < 12; i++) {
        float product = dot(reference_cell->floor_norms[i], -player_up);
        if (product < min) {
            min = product;
            min_idx = i;
        }
    }
    reference_cell->doors[min_idx] = reference_cell; 
        // Prevent the floor from becoming a door.
        // NOTE: this does NOT create an infinte looping paradox <3
    return min_idx;
};
void PlayerLocation::setFloorIndex(int index) {
    floor_indx = index;
}
void PlayerLocation::updateFocus(float x, float y, float dt) {
    //For now this is 'normalized', but hypothetically,
    //focus could permit DOF changes...
    vec3 vertical   = normalize(-player_up);
    vec3 horizontal = normalize(cross(vertical, focus));
    
    float dx = (x-mx < 0.1) ? x-mx : 0.1;
    float dy = (y-my < 0.1) ? y-my : 0.1;

    focus = normalize(normalize(focus) + (dx*horizontal*mouse_scale*dt) + (dy*vertical*mouse_scale*dt));

    mx = x;
    my = y;
};
//TODO: Use more modern "track ball" approach? 
//(Current implementation behaves weird lookging up or down too far)
//https://github.com/RockefellerA/Trackball/blob/master/Trackball.cpp
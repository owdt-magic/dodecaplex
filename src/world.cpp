#include "world.h"
#include "debug.h"

using namespace glm;

PlayerContext::PlayerContext() {
    if (player_location == NULL) {
        player_location = new PlayerLocation();
    }
    srand(time(NULL)); // Randomize the map...
};
std::size_t PlayerContext::initializeDodecaplexStates(GLuint* index_buffer){
    int dest = 0,
        read = 0,
        n,m,o,p;
    bool load_cell[120] = {false};
    
    for (int i =0; i < 12; i++) {
        n = neighbor_side_orders[i];
        load_cell[n] = !(rand()%2);
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

PlayerLocation::PlayerLocation() {
    if (reference_cell == NULL) reference_cell = new WorldCell();
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
void PlayerLocation::teleportHead(vec3 target) {
    head = target;
};
void PlayerLocation::teleportPUp(vec3 target) {
    player_up = target;
};
glm::mat4 PlayerLocation::getModel(){
    accumulated_transforms = displacement_transform*orientation_transform*accumulated_transforms;
    return accumulated_transforms;
};
void PlayerLocation::updatePosition(std::array<bool, 4> WASD, float dt) {
    static vec2 disp = vec2(0.0f);
    if (WASD[0]) disp.y -= dt;
    if (WASD[1]) disp.x -= dt;
    if (WASD[2]) disp.y += dt;
    if (WASD[3]) disp.x += dt;
    normalize(disp);
    disp *= movement_scale;
    displacement_transform = mat4({
        cos(disp.x), 0.0f, 0.0f, sin(disp.x), 
        0.0f,     1.0f, 0.0f, 0.0f, 
        0.0f,     0.0f, 1.0f, 0.0f,     
       -sin(disp.x), 0.0f, 0.0f, cos(disp.x)
    });
    displacement_transform *= mat4({
        1.0f, 0.0f,  0.0f,     0.0f, 
        0.0f, 1.0f,  0.0f,     0.0f, 
        0.0f, 0.0f,  cos(disp.y), sin(disp.y), 
        0.0f, 0.0f, -sin(disp.y), cos(disp.y)
    });
};
void PlayerLocation::updateFocus(float x, float y, float dt) {
    float dx = (x-mx < 0.1) ? x-mx : 0.1;
    float dy = (y-my < 0.1) ? y-my : 0.1;
    dy*=mouse_scale*dt;
    dx*=mouse_scale*dt;
    orientation_transform = mat4({
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cos(dy), sin(dy), 0.0f,
        0.0f, -sin(dy), cos(dy), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    });
    orientation_transform *= mat4({
        cos(dx), 0.0f, -sin(dx), 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sin(dx), 0.0f, cos(dx), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f

    });
    mx = x;
    my = y;
};
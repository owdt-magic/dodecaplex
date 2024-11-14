#include "world.h"
#include "debug.h"

using namespace glm;

void MapData::establishMap(){
    for (int i = 0; i < 120; i++) {
        load_cell[i] = rand()%2;
    } 
    load_cell[0] = true;
    load_cell[neighbor_side_orders[0]] = false;
};

PlayerContext::PlayerContext() {
    if (player_location == NULL) {
        player_location = new PlayerLocation();
    }
    srand(time(NULL)); // Randomize the map...
};
void PlayerContext::initializeWorldData(){
    map_data.establishMap();
    dodecaplex_vao = VAO((GLfloat*) &dodecaplex_cell_verts, sizeof(GLfloat)*600*4);
};
std::size_t PlayerContext::populateDodecaplexIndexBuffer(GLuint* index_buffer){
    int dest = 0,
        read = 0;

    for (int ci = 0; ci < 120; ci++) {
        if (map_data.load_cell[ci]) {
            for (int ord_idx = ci*12; ord_idx < ci*12 + 12; ord_idx++){
                if (!map_data.load_cell[neighbor_side_orders[ord_idx]]) {
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
void PlayerContext::indexDodecaplexVAOs() {
    std::size_t index_max_size   = 120*36*3*sizeof(GLuint);
    GLuint* index_buffer         = (GLuint*) malloc(index_max_size);
    std::size_t index_real_size  = populateDodecaplexIndexBuffer(index_buffer);
    
    dodecaplex_vao.NewIndeces((GLuint*) index_buffer, index_real_size);
    dodecaplex_vao.LinkAttrib(dodecaplex_vao.vbo, 0, 4, GL_FLOAT, 4*sizeof(float), (void*)0);

    free(index_buffer);

};
void PlayerContext::drawAllVAOs() {
    dodecaplex_vao.DrawElements(GL_TRIANGLES);
    for (int i = 0; i < additional_vaos.size(); i++){
        additional_vaos[i].DrawElements(GL_TRIANGLES);
    }
};
mat4 PlayerContext::getModelMatrix(std::array<bool, 4> WASD, float mouseX, float mouseY, float dt) {
    if (!player_location->overridden) {
        player_location->focusFromMouse(mouseX, mouseY, dt);
        player_location->positionFromKeys(WASD, dt);
        return player_location->getModel(map_data);
    } else {
        return player_location->elapseAnimation(dt);
    }
    
}

PlayerLocation::PlayerLocation() {
    if (reference_cell == NULL) reference_cell = new WorldCell();
    accumulated_transforms = mat4({
        1.0f, 0.0f,  0.0f,  0.0f, 
        0.0f, cos(height), 0.0f, sin(height), 
        0.0f, 0.0f,  1.0f, 0.0f, 
        0.0f, -sin(height), 0.0f, cos(height)
    })*getGravity(mat4(1.0f));
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
int PlayerLocation::getCellIndex() {
    return cell_index;
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
mat4 PlayerLocation::getModel(MapData map_data){
    int new_cell_index;
    vec4 new_cell_centroid;
    const vec4 origin = vec4(0.0f,0.0f,0.0f,1.0f);
    float max_dot, new_dot = 0.0f;
    float new_distance;
    mat4 potential_transform;
    potential_transform = displacement_transform*yaw_transform*accumulated_transforms;
    
    float old_distance = length(accumulated_transforms*cell_centroid - origin);
    for (int i = 0; i < 12; i++){
        new_cell_index = neighbor_side_orders[cell_index*12+i];
        new_cell_centroid = vec4(   dodecaplex_centroids[new_cell_index*4],
                                    dodecaplex_centroids[new_cell_index*4+1],
                                    dodecaplex_centroids[new_cell_index*4+2],
                                    dodecaplex_centroids[new_cell_index*4+3]);
        new_distance = length(potential_transform*new_cell_centroid - origin);
        if (new_distance < old_distance) {                
            if (!map_data.load_cell[new_cell_index] && !noclip) {
                accumulated_transforms = yaw_transform*accumulated_transforms;
                potential_transform = accumulated_transforms;
            } else {
                cell_index = new_cell_index;
                cell_centroid = new_cell_centroid;
            }
            break;
        }
    }
    accumulated_transforms = potential_transform;
    return pitch_transform*accumulated_transforms;
};
mat4 PlayerLocation::getGravity(mat4 current_transform){
    if (noclip) return mat4(1.0f);
    static float pit, rol;
    vec4 floor_normal = current_transform*(cell_centroid-floor_centroid);
    mat4 gravity_transform;
    
    pit = -std::atan2(floor_normal.z, floor_normal.y);
    gravity_transform=mat4({
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cos(pit), sin(pit), 0.0f,
        0.0f, -sin(pit), cos(pit), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    });
    floor_normal = gravity_transform*floor_normal;
    rol = std::atan2(floor_normal.x, floor_normal.y);
    return mat4({
        cos(rol), sin(rol), 0.0f, 0.0f,
        -sin(rol), cos(rol), 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    })*gravity_transform;
}
void PlayerLocation::positionFromKeys(std::array<bool, 4> WASD, float dt) {
    float dx = 0.0f, dy = 0.0f;
    if (WASD[0]) dy -= dt;
    if (WASD[1]) dx -= dt;
    if (WASD[2]) dy += dt;
    if (WASD[3]) dx += dt;
    dx *= movement_scale;
    dy *= movement_scale;
    updatePosition(vec3(dx, dy, 0.0f));
};
void PlayerLocation::updatePosition(vec3 d) {
    displacement_transform = mat4({
        cos(d.x), 0.0f, 0.0f, sin(d.x), 
        0.0f,     1.0f, 0.0f, 0.0f, 
        0.0f,     0.0f, 1.0f, 0.0f,     
       -sin(d.x), 0.0f, 0.0f, cos(d.x)
    });
    displacement_transform *= mat4({
        1.0f, 0.0f,  0.0f,     0.0f, 
        0.0f, 1.0f,  0.0f,     0.0f, 
        0.0f, 0.0f,  cos(d.y), sin(d.y), 
        0.0f, 0.0f, -sin(d.y), cos(d.y)
    });
    displacement_transform *= mat4({
        1.0f,  0.0f,     0.0f, 0.0f, 
        0.0f,  cos(d.z), 0.0f, sin(d.z), 
        0.0f,  0.0f,     1.0,  0.0f, 
        0.0f, -sin(d.z), 0.0f, cos(d.z)
    });
};
void PlayerLocation::focusFromMouse(float x, float y, float dt){
    float dx = abs(x-mx) < 0.1 ? (x-mx)*mouse_scale*dt : 0.1;
    float ty;
    if (noclip) {
        ty = abs(y-my) < 0.1 ? (y-my)*mouse_scale*dt : 0.1;
    } else {
        ty = tanh(y*2.0f)*3.1415f/2.0f;    
    }
    updateFocus(vec3(ty, dx, 1.0f));
    mx = x;
    my = y;
};
void PlayerLocation::updateFocus(vec3 r) {
    pitch_transform = mat4({
        1.0f,  0.0f,     0.0f,     0.0f,
        0.0f,  cos(r.x), sin(r.x), 0.0f,
        0.0f, -sin(r.x), cos(r.x), 0.0f,
        0.0f,  0.0f,     0.0f,     1.0f
    });    
    yaw_transform = mat4({
        cos(r.y), 0.0f, -sin(r.y), 0.0f,
        0.0f,     1.0f,  0.0f,     0.0f,
        sin(r.y), 0.0f,  cos(r.y), 0.0f,
        0.0f,     0.0f,  0.0f,     1.0f
    });
    roll_transform = mat4({
        cos(r.z), sin(r.z), 0.0f, 0.0f,
       -sin(r.z), cos(r.z), 0.0f, 0.0f,
        0.0f,     0.0f,     1.0f, 0.0f,
        0.0f,     0.0f,     0.0f, 1.0f
    });
    if (noclip) {
        // NOTE: when they are compounded into the
        // yaw transform, this makes it influence
        // the player's posision as it accumulates, thus,
        // allowing off plane displacement (flying/noclip)
        yaw_transform = pitch_transform*yaw_transform;
        pitch_transform = mat4(1.0f);
    }
};
void PlayerLocation::setAnimation(AnimationInfo* changes) {
    overridden = true;
    animation_info = changes;
};
mat4 PlayerLocation::elapseAnimation(float dt){
    float r;
    animation_info->progress += dt;
    if (animation_info->progress >= animation_info->durration) {
        overridden = false;
    }
    r = animation_info->progress/animation_info->durration;
    updateFocus(animation_info->rotations*r);
    updatePosition(animation_info->offsets*r);
    return displacement_transform*roll_transform*pitch_transform*yaw_transform*accumulated_transforms;
}
#include "playerLocation.h"

using namespace glm;

PlayerLocation::PlayerLocation() {
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
void PlayerLocation::teleportHead(vec3 target) {
    head = target;
};
void PlayerLocation::teleportPUp(vec3 target) {
    player_up = target;
};
mat4 PlayerLocation::currentTransform(){
    return pitch_transform*accumulated_transforms;
};
mat4 PlayerLocation::getModel(bool* loaded_cells){
    int new_cell_index;
    vec4 new_cell_centroid;
    const vec4 origin = vec4(0.0f,0.0f,0.0f,1.0f);
    float max_dot, new_dot = 0.0f;
    float new_distance;
    mat4 potential_transform;
    potential_transform = displacement_transform*yaw_transform*accumulated_transforms;
    
    float old_distance = length(accumulated_transforms*cell_centroid - origin);
    for (int i = 0; i < 12; i++){
        new_cell_index      = neighbor_side_orders[cell_index*12+i];
        new_cell_centroid   = dodecaplex_centroids[new_cell_index];
        new_distance        = length(potential_transform*new_cell_centroid - origin);
        if (new_distance < old_distance) {                
            if (!loaded_cells[new_cell_index] && !noclip) {
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
    
    pit = -atan2(floor_normal.z, floor_normal.y);
    gravity_transform=mat4({
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cos(pit), sin(pit), 0.0f,
        0.0f, -sin(pit), cos(pit), 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    });
    floor_normal = gravity_transform*floor_normal;
    rol = atan2(floor_normal.x, floor_normal.y);
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
};
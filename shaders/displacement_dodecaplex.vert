#version 410 core

layout(location = 0) in vec4 model_verts;
//layout(location = 1) in vec3 text_coords;
layout(location = 1) in mat4 DISP;

//uniform sampler2D depthTexture;
uniform mat4 CAMERA;
uniform mat4 WORLD;
uniform float u_time;

out vec3 surfaceNorm;
out vec4 world_position;
out float zDepth;

void main(){
    vec4 world_Pos = mix(DISP*model_verts, model_verts, (sin(u_time)+1)/2);//DISP * vec4(model_verts, 1.0); // Cell DISPlacement
    world_Pos = vec4(world_Pos.xyz/pow((world_Pos.w+sqrt(5)),3.0)*10, 1.0);
    //world_Pos = vec4(world_Pos.xyz, 1.0);

    world_position = world_Pos;
    world_Pos = WORLD * world_Pos; // Player movement in WORLD
    gl_Position = CAMERA * world_Pos; // Player camera
    zDepth = gl_Position.z;
}
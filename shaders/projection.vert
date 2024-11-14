#version 410 core
layout(location = 0) in vec4 model_verts;

uniform mat4 CAMERA;
uniform mat4 WORLD;

uniform float u_time;

out vec3 surfaceNorm;   
out vec4 mCoords;
out vec4 wCoords;
out float vDepth;

#define FOCUS sqrt(5)
#define PHI ( 1. + sqrt(5) ) / 2.

float ratio;

void project(inout vec4 vert){
    vert.xyz *= FOCUS/(FOCUS + vert.w);
    vert.w = 1.0;
}

void main(){
    vec4 world_Pos = WORLD*model_verts;
    mCoords = world_Pos;
    project(world_Pos);
    //mCoords = model_verts*3; // Temporary for color...
    gl_Position = CAMERA * world_Pos;
    vDepth = gl_Position.z;
    wCoords = world_Pos;
}
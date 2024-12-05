#version 410 core
layout(location = 0) in vec4 model_verts;
layout(location = 1) in vec3 model_textures;

uniform mat4 CAMERA;
uniform mat4 WORLD;

uniform float u_time;

out vec3 surfaceNorm;   
out vec4 mCoords;
out vec4 wCoords;
out float vDepth;
out vec3 tCoords;

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
    gl_Position = CAMERA * world_Pos;
    vDepth = gl_Position.z;
    wCoords = world_Pos;
    tCoords = model_textures;
}
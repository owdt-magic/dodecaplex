#version 410 core
layout(location = 0) in vec4 model_verts;
layout(location = 1) in vec3 model_textures;

uniform mat4 CAMERA;
uniform mat4 WORLD;

uniform float u_time;

out vec4 mCoords;
out vec4 wCoords;
out vec3 tCoords;

#define FOCUS sqrt(5)
#define PHI ( 1. + sqrt(5) ) / 2.

float ratio;

void project(inout vec4 vert){
    vert.xyz *= FOCUS/(FOCUS + vert.w);
    vert.w = 1.0;
}

void main(){
    vec4 temp = WORLD * model_verts;
    mCoords = temp;
    project(temp);
    wCoords = temp;
    tCoords = model_textures;
    gl_Position = CAMERA * temp;
}
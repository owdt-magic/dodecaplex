#version 410 core
layout(location = 0) in vec4 model_verts;
layout(location = 1) in vec3 model_textures;

#include projection.glsl

void main(){
    processVerts(model_verts);
}
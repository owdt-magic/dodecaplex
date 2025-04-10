#include global.glsl

layout(location = 0) in vec4 model_verts;
layout(location = 1) in vec3 model_textures;
layout(location = 2) in vec4 shrapnel_offset;

uniform float SPELL_LIFE;

#include projection.glsl

void main(){
    processVerts(model_verts+shrapnel_offset*3.0*(1.0-SPELL_LIFE));
}
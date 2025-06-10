#include global.glsl

layout(location = 0) in vec4 model_verts;
layout(location = 1) in vec3 model_textures;

#include projection.glsl

 uniform vec4 u_audio_bands;

void main(){
    mat4 supplemental_rotation = mat4(1.0);
    vec4 bands = u_audio_bands/200.0;
    supplemental_rotation *= mat4( 
        1.0,  0.0,    0.0, 0.0,
        0.0,  cos(bands.x), 0.0, sin(bands.x),
        0.0,  0.0,    1.0, 0.0,
        0.0, -sin(bands.x), 0.0, cos(bands.x)
    );
    supplemental_rotation *= mat4(
        cos(bands.y), 0.0, 0.0, sin(bands.y), 
        0.0,     1.0, 0.0, 0.0, 
        0.0,     0.0, 1.0, 0.0,     
       -sin(bands.y), 0.0, 0.0, cos(bands.y)
    );
    supplemental_rotation *= mat4(
        1.0, 0.0,  0.0,     0.0, 
        0.0, 1.0,  0.0,     0.0, 
        0.0, 0.0,  cos(bands.z), sin(bands.z), 
        0.0, 0.0, -sin(bands.z), cos(bands.z)
    );

    supplemental_rotation *= mat4(
        1.0,  0.0,     0.0,     0.0,
        0.0,  cos(bands.y), sin(bands.y), 0.0,
        0.0, -sin(bands.y), cos(bands.y), 0.0,
        0.0,  0.0,     0.0,     1.0
    );    
    supplemental_rotation *= mat4(
        cos(bands.z), 0.0, -sin(bands.z), 0.0,
        0.0,     1.0,  0.0,     0.0,
        sin(bands.z), 0.0,  cos(bands.z), 0.0,
        0.0,     0.0,  0.0,     1.0
    );
    supplemental_rotation *= mat4(
        cos(bands.w), sin(bands.w), 0.0, 0.0,
       -sin(bands.w), cos(bands.w), 0.0, 0.0,
        0.0,     0.0,     1.0, 0.0,
        0.0,     0.0,     0.0, 1.0
    );
    processVerts(supplemental_rotation*model_verts);
}
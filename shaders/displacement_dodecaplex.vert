#version 410 core
layout(location = 0) in vec4 model_verts;

uniform mat4 CAMERA;
uniform mat4 WORLD;

uniform mat4 MATRIX_0;
uniform mat4 MATRIX_1;
uniform mat4 MATRIX_2;
uniform vec4 AXIS_0;
uniform vec4 AXIS_1;
uniform vec4 AXIS_2;
uniform vec3 HEAD;

uniform float u_time;

out vec3 surfaceNorm;
out vec4 world_Coords;
out float zDepth;

#define FOCUS sqrt(5)
#define PHI ( 1. + sqrt(5) ) / 2.

float ratio;

mat3 rebaseMatrix(in vec4 v0, in vec4 v1, in vec4 v2) {
    //float divisor = (v0.x*v1.y*v2.z - v0.x*v1.z*v2.y - v0.y*v1.x*v2.z + v0.y*v1.z*v2.x + v0.z*v1.x*v2.y - v0.z*v1.y*v2.x);
    //NOTE: Technically, divisor is superfluous and can be skipped...
    return mat3(
        (v1.y*v2.z - v1.z*v2.y), (-v1.x*v2.z + v1.z*v2.x), (v1.x*v2.y - v1.y*v2.x),
        (-v0.y*v2.z + v0.z*v2.y), (v0.x*v2.z - v0.z*v2.x), (-v0.x*v2.y + v0.y*v2.x),
        (v0.y*v1.z - v0.z*v1.y), (-v0.x*v1.z + v0.z*v1.x), (v0.x*v1.y - v0.y*v1.x)
    );//divisor;
}

mat4 rotationMatrix(in vec3 axis){
    axis = normalize(axis);
    return mat4(
        2.*axis.x*axis.x-1.,  2.*axis.y*axis.x,      2.*axis.z*axis.x,        0.,
        2.*axis.x*axis.y,       2.*axis.y*axis.y-1., 2.*axis.z*axis.y,        0.,
        2.*axis.x*axis.z,       2.*axis.y*axis.z,      2.*axis.z*axis.z-1.,   0.,
        0.,                   0.,                  0.,                    1.
    );
}

vec4 modelToWorld(in vec4 model) {
    vec3 head = atan(HEAD);
    vec4 transform_0 = (rotationMatrix(AXIS_0.xyz)*MATRIX_0)*model;
    vec4 transform_1 = (rotationMatrix(AXIS_1.xyz)*MATRIX_1)*model;
    vec4 transform_2 = (rotationMatrix(AXIS_2.xyz)*MATRIX_2)*model;
    vec3 weights = head*rebaseMatrix(AXIS_0, AXIS_1, AXIS_2);
    weights /= (weights.x + weights.y + weights.z);
    
    ratio = length( head ) / (0.25 + 0.25*PHI*PHI);
    ratio = clamp(ratio, 0.0, 1.0);
    return mix(model,
        (transform_0*weights.x) + (transform_1*weights.y) + (transform_2*weights.z),
        ratio
    );

}

void project(inout vec4 vert){
    vert.xyz *= FOCUS/(FOCUS + vert.w);
    vert.w = 1.0;
}

void main(){
    vec4 world_Pos = modelToWorld(model_verts);
    project(world_Pos);
    world_Coords = model_verts*3; // Temporary for color...
    gl_Position = CAMERA * world_Pos;
    zDepth = gl_Position.z;
}
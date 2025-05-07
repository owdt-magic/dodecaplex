#version 410 core

layout(location = 0) in vec3 model_verts;
layout(location = 1) in mat4 DISP;

uniform mat4 MVP;

out vec3 fragmentColor;

float rand(vec2 co){
    return fract(sin(dot(co, vec2(12.9898, 78.233))) * 43758.5453);
}

void main(){
    // The color of each vertex will be interpolated
    // to produce the color of each fragment

    fragmentColor = vec3(rand(model_verts.xy), rand(model_verts.xz), rand(model_verts.yz));
    gl_Position = MVP * DISP * vec4(model_verts, 1.0);
}
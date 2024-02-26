#version 410 core

layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec3 vertexColor;

out vec3 fragmentColor;

void main(){
    // The color of each vertex will be interpolated
    // to produce the color of each fragment
    fragmentColor = vertexColor;
    gl_Position.xyz = vertexPosition_modelspace;
    gl_Position.w = 1.0;
}
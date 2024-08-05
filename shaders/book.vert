#version 330 core

layout(location = 0) in vec3 aPos; // Vertex position
layout(location = 1) in vec3 aTexCoord; // Texture coordinate

uniform float u_time;

out vec3 TexCoord; // Output texture coordinate to the fragment shader

void main() {
    float flip_progress = u_time;
    vec3 pagePos = aPos;
    pagePos.y = -pagePos.y;
    pagePos.x = pagePos.x*cos(flip_progress);
    pagePos.y += abs(sin(pagePos.x*3.1415)/20.);

    gl_Position = vec4(pagePos, 1.0);
    TexCoord = aTexCoord;
}
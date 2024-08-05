#version 330 core

layout(location = 0) in vec3 aPos; // Vertex position
layout(location = 1) in vec3 aTexCoord; // Texture coordinate

out vec3 TexCoord; // Output texture coordinate to the fragment shader

void main() {
    gl_Position = vec4(aPos, 1.0);
    TexCoord = aTexCoord;
}
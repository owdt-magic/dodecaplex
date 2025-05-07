#version 410 core

in vec3 fragmentColor;

out vec4 color;

void main(){
    // Output color = color specified in the vertex shader,
    // interpolated between all 3 surrounding vertices
    color = vec4(fragmentColor, 1.0);
}

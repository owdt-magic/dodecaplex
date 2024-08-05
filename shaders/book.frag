
#version 330 core

in vec3 TexCoord; // Input texture coordinate from the vertex shader
out vec4 FragColor; // Output fragment color

uniform sampler2DArray grimoireTextures; // Texture array

void main() {
    // Sample the texture at the given texture coordinate and layer 0
    FragColor = texture(grimoireTextures, TexCoord);
}
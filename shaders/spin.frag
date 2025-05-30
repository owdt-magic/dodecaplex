#version 410 core

in float zDepth;
in vec4 model_Coords;
in vec3 texture_Coords;

uniform float u_time;
uniform vec2 u_resolution;
uniform vec4 u_audio_bands;

out vec4 color;

void main(){
    color = u_audio_bands/5.0;
    color = vec4(color.x/4.0, color.yw, 1.0);
    color /= max(zDepth*1.5, 1.0);
}

#version 410 core

in float zDepth;
in vec4 model_Coords;
in vec3 texture_Coords;

uniform float u_time;
uniform vec2 u_resolution;
uniform vec4 u_audio_bands;
uniform float u_brightness;

out vec4 color;

void main(){
    color = max(u_audio_bands, vec4(u_brightness));
    color /= max(zDepth*1.0, 1.0);
}

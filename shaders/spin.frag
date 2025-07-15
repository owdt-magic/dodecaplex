#version 410 core

in float zDepth;
in vec4 model_Coords;
in vec3 texture_Coords;

uniform float u_time;
uniform float u_scale;
uniform vec2 u_resolution;
uniform vec4 u_audio_bands;
uniform float u_brightness;
uniform float u_hueShift;
uniform float u_vignette;
#include hueRotation.glsl

out vec4 color;

void main(){
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution;
    
    color = u_audio_bands*vec4(u_brightness) + 0.1*vec4(u_brightness);
    color /= max(zDepth*1.0, 1.0);
    color = color*(1.0-pow(length(uv)/1.2, u_vignette));
    color.rgb = hueShift(color.rgb, u_hueShift);
    color.rgb = pow(color.rgb, 1.0f-vec3(u_scale));
}

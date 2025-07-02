#version 330 core
#include hg_sdf.glsl
layout (location = 0) out vec4 fragColor;

uniform vec2 u_resolution;
uniform float u_time;

uniform float u_band0;
uniform float u_band1;
uniform float u_band2;
uniform float u_band3;

void main() {
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution.y;
    vec4 bands = vec4(u_band0, u_band1/2.0, u_band2, u_band3);
    fragColor = mat4(
        sin(uv.x), cos(uv.x), 0.0, 1.0,
        -cos(uv.x), -sin(uv.x), 0.0, 1.0,
        0.0, 0.0, sin(uv.y), cos(uv.y),
        0.0, 0.0, cos(uv.y), -sin(uv.y)
    )*pow(bands, vec4(0.5));
    
}
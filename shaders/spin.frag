#version 410 core

in float zDepth;
in vec4 model_Coords;
in vec3 texture_Coords;
in vec3 bary_Coords;
flat in vec2 wP0, wP1, wP2;

uniform float u_time;
uniform float u_scale;
uniform vec2 u_resolution;
uniform vec4 u_audio_bands;
uniform float u_brightness;
uniform float u_hueShift;
uniform float u_vignette;
uniform float u_linePx;
uniform float u_lineFade;
#include hueRotation.glsl

out vec4 color;

float edgeDistPx(vec2 p, vec2 a, vec2 b) {
    vec2 e = b - a;
    float elen = length(e);
    // Perpendicular unit normal to edge
    vec2 n = (elen > 1e-6) ? vec2(-e.y, e.x) / elen : vec2(0.0, 1.0);
    // Signed distance from point to infinite line through ab
    return abs(dot(p - a, n));
}

void main(){
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution;
    
    vec4 base = u_audio_bands * vec4(u_brightness) + 0.1 * vec4(u_brightness);
    base /= max(zDepth * 1.0, 1.0);
    base *= (1.0 - pow(length(uv)/1.2, u_vignette));
    base.rgb = hueShift(base.rgb, u_hueShift);
    base.rgb = pow(base.rgb, 1.0 - vec3(u_scale));

    vec2 p  = gl_FragCoord.xy;
    float d0 = edgeDistPx(p, wP0, wP1);
    float d1 = edgeDistPx(p, wP1, wP2);
    float d2 = edgeDistPx(p, wP2, wP0);
    float d  = min(d0, min(d1, d2));

    float t = u_linePx/max(pow(zDepth, 0.5), 1.0);
    float s = max(u_lineFade, 0.0001);

    float edge = 1.0 - smoothstep(t, t + s, d);

    color = vec4(base.rgb, edge);
    if (color.a <= 0.001) discard;
}

#version 330 core
layout (location = 0) out vec4 fragColor;

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;
uniform float u_scroll;

#include camera.glsl

#define DUNE_SIZE 3.6

vec2 hash22(vec2 p) {
    p = fract(p * vec2(5.3983, 5.4427));
    p += dot(p, p + 21.5351);
    return fract(vec2(p.x * p.y, p.x + p.y));
}

float gradN2D(vec2 p) {
    vec2 f = fract(p), i = floor(p), w = f*f*(3. - 2.*f);
    vec2 e = vec2(0., 1.);

    float a = dot(hash22(i + e.xx), f - e.xx);
    float b = dot(hash22(i + e.yx), f - e.yx);
    float c = dot(hash22(i + e.xy), f - e.xy);
    float d = dot(hash22(i + e.yy), f - e.yy);

    return mix(mix(a, b, w.x), mix(c, d, w.x), w.y) * 0.5 + 0.5;
}

float fBm(vec2 p) {
    return gradN2D(p)*0.57 + gradN2D(p*2.0)*0.28 + gradN2D(p*4.0)*0.15;
}

float grad(float x, float offs) {
    x = abs(fract(x / 6.283 + offs - 0.25) - 0.5) * 2.0;
    float pike = clamp(x * x * (-1. + 2. * x), 0., 1.);
    return mix(smoothstep(0., 1., x), pike, 0.15);
}

float sdDune(vec3 p) {
    float y = fBm(vec2(p.x, p.z+u_time));
    //y += fBm(vec2(p.x, p.z-2.0*u_time)/3.14)/2.;
    // Wavy ^?
    return p.y-y;
}

float rayMarch(vec3 ro, vec3 rd) {
    float t = 0.0;
    for (int i = 0; i < 100; i++) {
        vec3 p = ro + rd * t;
        float d = sdDune(p);
        if (abs(d) < 0.001) break;
        t += d;
        if (t > 100.0) break;
    }
    return t;
}

vec3 getSandColor(vec3 worldPos) {
    float height = worldPos.y*-100.0;

    // Approximate slope: higher slope = darker, shadowed sand
    //float slope = clamp(, 0.0, 1.0);
    float dx = dFdx(height);
    float dy = dFdy(height);
    float slope = dx*dx+dy*dy;

    // Base sand colors
    vec3 sandLight = vec3(0.96, 0.84, 0.61); // Sunlit
    vec3 sandDark  = vec3(0.62, 0.51, 0.33); // Shadow

    // Blend based on slope
    vec3 baseColor = mix(sandLight, sandDark, pow(slope,0.1));

    // Optional height tinting: higher dunes slightly lighter
    // float heightTint = smoothstep(0.0, 1.0, height/10.0);
    // baseColor += vec3(0.05, 0.04, 0.03) * heightTint;

    return baseColor;
}


const float FOV = 1.0;
void main() {
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution.y;
    
    vec3 ro     = vec3(0.0, 0.1, 0.0)/abs(u_scroll/10.0);
    vec3 lookAt = vec3(0, 0.0, 100.);
    mouseControl(lookAt);
    vec3 rd = getCam(ro, lookAt) * normalize(vec3(uv, FOV));

    float t = rayMarch(ro, rd);
    vec3 worldPos = ro + rd * t;
    
    vec3 color = getSandColor(worldPos);
    fragColor = vec4(color, 1.0);
}

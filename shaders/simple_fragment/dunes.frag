#version 330 core
layout (location = 0) out vec4 fragColor;

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;
uniform float u_scroll;

#include camera.glsl
#include sharedUniforms.glsl

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

    //return mix(mix(a, b, w.x), mix(c, d, w.x), w.y) * 0.5 + 0.5;
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
    float time = u_time * u_speed;
    float y = fBm(vec2(p.x, p.z+time));
    //y += fBm(vec2(p.x, p.z-2.0*u_time)/3.14)/2.;
    // Wavy ^?
    return p.y-y;   
}

float rayMarch(vec3 ro, vec3 rd) {
    float t = 0.0;
    float d;
    vec3 p;
    for (int i = 0; i < 50; i++) {
        p = ro + rd * t;
        d = sdDune(p);
        if (abs(d) < 0.00001) break;
        t += d;
        if (t > 10.0){
            //t = 100000.;
            break;
        }
    }
    return t;
}

vec3 getSandColor(vec3 worldPos, float slope) {
    float height = worldPos.y*-100.0;

    // Base sand colors
    vec3 sandLight = vec3(0.96, 0.84, 0.61); // Sunlit
    vec3 sandDark  = vec3(0.62, 0.51, 0.33); // Shadow

    // Blend based on slope
    vec3 baseColor = mix(sandLight, sandDark, pow(slope,0.1));

    // Optional height tinting: higher dunes slightly lighter
    float heightTint = smoothstep(0.0, 100.0, -height);
    baseColor *= vec3(0.5, 0.4, 0.3) * heightTint*(1.0+u_scale*4.0);
    
    return pow(baseColor, vec3(u_brightness));
}

vec3 getSurface(vec3 ro, vec2 uv, mat3 camera) {
    const float delta = 0.001;
    vec3 rd = camera * normalize(vec3(uv, 45./u_fov));
    float t = rayMarch(ro, rd);
    vec3 worldPos = ro + rd * t;

    rd = camera * normalize(vec3(uv+vec2(delta, 0.), 45./u_fov));
    float dx = rayMarch(ro, rd);
    rd = camera * normalize(vec3(uv+vec2(0., delta), 45./u_fov)); 
    float dy = rayMarch(ro, rd);    
    dx -= t;
    dy -= t;
    float slope = (dx*dx+dy*dy)*u_brightness;
    
    vec3 color = getSandColor(worldPos, slope);
    return color;
}


void main() {
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution.y;
    
    vec3 ro     = vec3(0.0, 1.0, 0.0)/abs(u_scroll);
    vec3 lookAt = vec3(0, 0.0, 100.);
    mouseControl(lookAt);
    
    mat3 cam = getCam(ro, lookAt);
    vec3 color = getSurface(ro, uv, cam);
    
    fragColor = vec4(color, 1.0);
}

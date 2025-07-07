#version 330 core
#include hg_sdf.glsl
layout (location = 0) out vec4 fragColor;

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;
uniform float u_scroll;
#include sharedUniforms.glsl
#include hueRotation.glsl

const float FOV = 0.8;
const int MAX_STEPS = 32;
const float MAX_DIST = 500;
const float EPSILON = 0.001;
const float VIEW_HEIGHT = 2.0;

vec2 radial_mod(in vec2 source, in float radius, in float arc_len){
    float angle = atan(-source.y, source.x);
    angle -= mod(angle+arc_len/2., arc_len) - arc_len/2.;
    source = source+vec2(-cos(angle), sin(angle))*radius;
    pR(source, -angle);
    return source;
}


float map(in vec3 from){
    float time = u_time*u_speed;
    float orig_x = from.x;
    float rot = from.z/4.0 + from.y/2.0;
    float offset = cross(from.xyz, vec3(0., 0., 1.)).x;
    float spacing = 1.5;
    
    pR(from.zy, rot); 
    pR(from.zx, -rot/10.); 
    
    pR(from.yz, (time/5.0));//+from.x);
    from.x = mod(from.x-time, spacing)-spacing/2.;
    from.yz = radial_mod(from.yz, VIEW_HEIGHT*(1.25+cos(time)/4.0), PI/5.);

    float angle = atan(from.y, from.x);
    
    pR(from.zy, 1.45*time+angle);
    pR(from.yx, angle);
    return max(fDodecahedron(from, (cos(time)+2.0, 10.)/1.5),fIcosahedron(from, .9));
}


float castRay( in vec3 ro, in vec3 rd) {
    float t = 0.;
    for( int i=0; i<MAX_STEPS; i++ ){
        float res = map( ro+rd*t);
        if (abs(res) < EPSILON)
            return t;
        t += res;
        if (t > MAX_DIST)
            break;
    }
    return 1000.;
}

vec3 getNormal(in vec3 from) {
    vec2 delta = vec2(0.01,0.0);
    vec3 direction = map(from) - 
        vec3(map(from-delta.xyy), map(from-delta.yxy), map(from-delta.yyx));
    return normalize(direction);
}


mat3 getCam(vec3 ro, vec3 lookAt) {
    vec3 camF = normalize(vec3(lookAt - ro));
    vec3 camR = normalize(cross(vec3(0, 1, 0), camF));
    vec3 camU = cross(camF, camR);
    return mat3(camR, camU, camF);
}

void mouseControl(inout vec3 ro) {
    pR(ro.xz, u_scroll * TAU);
}

vec3 render(in vec2 uv) {
    // Camera data
    vec3 col;
    vec3 ro = vec3(1.0, .00001 ,0.0);
    mouseControl(ro);
    vec3 lookAt = vec3(0., 0., 0.);
    vec3 rd = getCam(ro, lookAt) * normalize(vec3(uv, FOV));

    // Scene geometry
    float dist      = castRay(ro, rd);
    vec3 location   = ro+rd*dist;
    vec3 world_norm = getNormal(location);
    vec3 cam_norm   = normalize(world_norm*normalize(ro-lookAt));

    // Lights
    vec3 key_light      = vec3(0., 3.4, 7.4);
    float key_exposure  = dot(world_norm, normalize(key_light-location))/log(10.*length(key_light-location));
    vec3 glint_light    = vec3(0., 10., 0.);
    vec3 fill_light     = vec3(-10., 0., 0.);

    // Color Translations
    vec2 arc_info       = normalize(ro.yz+(rd*dist).yz);
    vec3 background     = vec3(0.);
    vec3 material_color = vec3(3., .4, 9.);//cross(world_norm,cam_norm);
    vec3 result_color   = material_color * (min(vec3(key_exposure), material_color)/2.);
    result_color = max(result_color, material_color*pow(dot(world_norm, fill_light/100.)+.1, 2.8));
    result_color = max(pow(result_color * dot(cam_norm, glint_light)*70000., vec3(1.9)), result_color);
    result_color = max(pow(result_color * dot(cam_norm, -glint_light)*70000., vec3(1.9)), result_color);
    
    if (dist < MAX_DIST) {
        col = mix(result_color, background, 1.0 - exp(-0.0005*dist*dist));
    } else {
        col = background;
    }
    return col;
}

void main() {
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution.y;
    
    // AA
    /*float delta = 0.001;
    vec3 col = (render(uv-vec2(0.0,delta))+render(uv+vec2(delta,0.0))
    +render(uv-vec2(delta,0.0))+render(uv+vec2(0.0,delta)))/4.0;*/
    vec3 col = render(uv);
    uv *= u_resolution.yy/u_resolution;
    
    // gamma correction
    col = pow(atan(col*u_brightness), vec3(u_scale+0.1));
    
    // vignette
    col = col*(1.0-pow(length(uv)/1.2, u_fov/100.));
    fragColor = vec4(hueShift(col, u_hueShift), 1.0);
}
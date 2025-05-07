#version 330 core
#include hg_sdf.glsl
layout (location = 0) out vec4 fragColor;

uniform vec2 u_resolution;
uniform vec2 u_mouse;
uniform float u_time;
uniform float u_scroll;

const float FOV = 1.0;
const int MAX_STEPS = 256;
const float MAX_DIST = 500.0;
const float EPSILON = 0.001;
const float ROOM_SCALE = 20.0;

mat3 getCam(vec3 ro, vec3 lookAt) {
    vec3 camF = normalize(vec3(lookAt - ro));
    vec3 camR = normalize(cross(vec3(0, 1, 0), camF));
    vec3 camU = cross(camF, camR);
    return mat3(camR, camU, camF);
}

void mouseControl(inout vec3 ro) {
    vec2 m = u_mouse / u_resolution;
    pR(ro.yz, m.y* PI);
    pR(ro.xz, m.x * TAU);
}

float map(in vec3 from){
    vec3 box_dims = vec3(ROOM_SCALE);
    return abs(vmax(abs(from)-box_dims)) -0.1;
    //return mix(abs(fBox(from, vec3(ROOM_SCALE))), abs(fSphere(from, ROOM_SCALE)), 0.9);
}

vec3 getNormal(in vec3 from) {
    vec2 delta = vec2(0.01,0.0);
    vec3 direction = map(from) - 
        vec3(map(from-delta.xyy), map(from-delta.yxy), map(from-delta.yyx));
    return normalize(direction);
}

float castRay( in vec3 ro, in vec3 rd) {
    float t = 0.;
    for( int i=0; i<MAX_STEPS; i++ ){
        float res = map( ro+rd*t);
        if (abs(res) < EPSILON)
            return t;
        t += res*.5;
        if (t > MAX_DIST)
            break;
    }
    return MAX_DIST;
}

float getAmbientOcclusion(vec3 p, vec3 normal) {
    float occ = 0.0;
    float weight = 2.0;
    for (int i = 0; i < 8; i++) {
        float len = 0.01 + 0.02 * float(i * i);
        float dist = map(p + normal * len);
        occ += len - dist * weight;
        weight *= 0.85;
    }
    return 1.0 - clamp(0.6 * occ, 0.0, 1.0);
}

void lightIt(inout vec3 col, in vec3 spec_col, in vec3 light_location, 
    in vec3 point, in vec3 rd, in vec4 config, in bool occlude, in float original_mix){
    
    // Physical light factors...
    vec3 light_distance = light_location-point;
    vec3 light_offset   = normalize(light_distance);
    vec3 norm           = getNormal(point);
    vec3 reflection     = reflect(-light_offset, norm);

    // Directional lighting factors...
    vec3 specular   = config.r * spec_col *pow(clamp(dot(reflection, -rd), 0.0, 1.0), 10.0);//log(length(light_distance)) ;
    vec3 diffuse    = config.g * col * clamp(dot(light_offset, norm), 0.0, 1.0);
    vec3 fresnel    = config.b * col * pow(1.0 + dot(rd, norm), 3.0);
    vec3 ambient    = config.a * col;
    vec3 back       = config.a * col * clamp(dot(norm, -light_offset), 0.0, 1.0);

    // Scaling factors...
    float shadow    = 1.0;//TODO: TEMPORAY
    if (occlude){
        float occ   = getAmbientOcclusion(point,  norm);
        col = mix((back + ambient + fresnel) * occ + (specular * occ + diffuse) * shadow, col, original_mix);
    } else{
        col = mix((back + ambient + fresnel) + (specular + diffuse) * shadow, col, original_mix);
    }
}

void ceilingLight(inout vec3 col, in vec3 point, in vec3 rd){
    vec4 light_scale_config = vec4(0.8, 0.9, 0.15, 0.10);
    vec3 spec_col = vec3(0.6, 0.5, 0.4);
    vec3 light_location = vec3(0.);//vec3(cos(u_time)*10.0, ROOM_SCALE*.9, sin(u_time)*10.0);
    lightIt(col, spec_col, light_location, point, rd, light_scale_config, true, 0.);
}

/*vec3 aura_map(in vec3 point, in vec3 aura_point){

return 
}

float castAura( in vec3 ro, in vec3 rd) {
    float t = 0.;
    for( int i=0; i<MAX_STEPS; i++ ){
        float res = aura_map( ro+rd*t);
        if (abs(res) < EPSILON)
            return t;
        t += res*.5;
        if (t > MAX_DIST)
            break;
    }
    return MAX_DIST;
}*/

void spellLight(inout vec3 col, in vec3 point, in vec3 rd, in vec3 aura_point, in float spell_progress){
    vec3 spec_col = vec3(0.9, 0.3, 0.2);
    vec4 light_scale_config = vec4(0.9, 0.9*log(2.0-spell_progress), 0.15*spell_progress, 0.5);

    lightIt(col, spec_col, aura_point, point, rd, light_scale_config, false, 1.0-spell_progress);
    /*float glow = 1./length(aura_point-point);
    col += glow/10.;*/
    float dist_factor = length(aura_point-point);
    col /= max(.1*log(dist_factor), .1);
}

vec3 projectOntoPlane(vec3 ray_direction, vec3 aura_direction) {
  // Ensure aura_direction is normalized
  vec3 n = normalize(aura_direction);
    
  // Project ray_direction onto the plane defined by aura_direction as the normal
  vec3 projection = ray_direction - dot(ray_direction, n) * n;
    
  return projection;
}


vec3 spellRing(in vec3 ro, in vec3 rd, in vec3 aura_point, in float progress){
    vec3 aura_direction = aura_point-ro;
    float to_surface = length(aura_direction);
    vec3 rd_intersection = ro + rd * to_surface;
    
    // Assuming aura_direction is normalized
    vec3 tempVector = vec3(0.,1.,0.);
    vec3 u = normalize(cross(aura_direction, tempVector)); // Generate one basis vector
    vec3 v = normalize(cross(aura_direction, u)); // Generate the second basis vector orthogonal to the first

    vec3 projection = projectOntoPlane(rd, aura_direction);
    
    float x = dot(projection, u);
    float y = dot(projection, v);
    
	vec2 planeProjection = vec2(x, y);
    
    float from_center = length(planeProjection);
      //length(rd_intersection-aura_point);
    float radius = .4*(5.0-5.0*sin(progress*(PI/2.)));
    float fall_off = 0.04;
    return smoothstep(radius-fall_off, radius, from_center)*
           smoothstep(radius+fall_off, radius, from_center)*
           vec3(1.);
}

vec3 render_room(in vec3 ro, in vec3 rd, in vec3 lookAt){
    float dist = castRay(ro, rd);
    vec3 background = vec3(0.);
    if (dist < MAX_DIST){
        vec3 point = ro+rd*dist;
        vec3 col = vec3(.4, .44, .4 )/2.0;

        // Spell variables...
        float spell_progress = (sin(u_time)/2.)+.5;
        float wall = castRay(ro-lookAt, normalize(lookAt-ro));
        vec3 aura_point = mix((ro-lookAt), normalize(lookAt-ro)*wall, spell_progress);
        spellLight(col, point, rd, aura_point, spell_progress);

        ceilingLight(col, point, rd);

        col = max(col, spellRing(ro, rd, aura_point, spell_progress));
        //col += ring_opacity;
        
        return col;
    } else {
        return background;
    }
}

void main() {
    vec2 uv = (2.0 * gl_FragCoord.xy - u_resolution.xy) / u_resolution.y;
    vec3 ro = vec3(15.0, 9.0,9.0) / u_scroll;
    mouseControl(ro);

    vec3 lookAt = vec3(0, 0, 0);
    vec3 rd = getCam(ro, lookAt) * normalize(vec3(uv, FOV));


    vec3 col = render_room(ro, rd, lookAt);

    

    // gamma correction
    col = pow(col, vec3(0.4545));
    fragColor = vec4(col, 1.0);
}

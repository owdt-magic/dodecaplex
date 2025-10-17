// Purple Vortex Shader - Shadertoy Version
// Converted from OpenGL shader

#define PI 3.14159265
#define TAU (2.0*PI)
#define PHI ((2.2360679775*0.5) + 0.5)

// Custom parameters (can be adjusted in Shadertoy)
float u_scale = 1.0;
float u_brightness = 1.0;
float u_speed = 1.0;
float u_fov = 50.0;
float u_hueShift = 0.0;


const vec3 GDFVectors[19] = vec3[](
	normalize(vec3(1, 0, 0)),
	normalize(vec3(0, 1, 0)),
	normalize(vec3(0, 0, 1)),

	normalize(vec3(1, 1, 1 )),
	normalize(vec3(-1, 1, 1)),
	normalize(vec3(1, -1, 1)),
	normalize(vec3(1, 1, -1)),

	normalize(vec3(0, 1, PHI+1.0)),
	normalize(vec3(0, -1, PHI+1.0)),
	normalize(vec3(PHI+1.0, 0, 1)),
	normalize(vec3(-PHI-1.0, 0, 1)),
	normalize(vec3(1, PHI+1.0, 0)),
	normalize(vec3(-1, PHI+1.0, 0)),

	normalize(vec3(0, PHI, 1)),
	normalize(vec3(0, -PHI, 1)),
	normalize(vec3(1, 0, PHI)),
	normalize(vec3(-1, 0, PHI)),
	normalize(vec3(PHI, 1, 0)),
	normalize(vec3(-PHI, 1, 0))
);

float fGDF(vec3 p, float r, int begin, int end) {
	float d = 0.0;
	for (int i = begin; i <= end; ++i)
		d = max(d, abs(dot(p, GDFVectors[i])));
	return d - r;
}

float fDodecahedron(vec3 p, float r) {
	return fGDF(p, r, 13, 18);
}


float fIcosahedron(vec3 p, float r) {
	return fGDF(p, r, 3, 12);
}

// Rotation function
void pR(inout vec2 p, float a) {
	p = cos(a)*p + sin(a)*vec2(p.y, -p.x);
}

// Hue shift utility function
vec3 hueShift(vec3 color, float shift) {
    float angle = radians(shift);
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    
    mat3 hue_matrix = mat3(
        cos_a + (1.0 - cos_a) / 3.0, (1.0 - cos_a) / 3.0 - sin_a / sqrt(3.0), (1.0 - cos_a) / 3.0 + sin_a / sqrt(3.0),
        (1.0 - cos_a) / 3.0 + sin_a / sqrt(3.0), cos_a + (1.0 - cos_a) / 3.0, (1.0 - cos_a) / 3.0 - sin_a / sqrt(3.0),
        (1.0 - cos_a) / 3.0 - sin_a / sqrt(3.0), (1.0 - cos_a) / 3.0 + sin_a / sqrt(3.0), cos_a + (1.0 - cos_a) / 3.0
    );
    
    return hue_matrix * color;
}

// Removed radial_mod function as it's no longer used

const float FOV = 0.8;
const int MAX_STEPS = 16; // Reduced from 32
const float MAX_DIST = 100.0; // Reduced from 500
const float EPSILON = 0.01; // Increased from 0.001
const float VIEW_HEIGHT = 2.0;

float map(in vec3 from){
    float time = iTime * u_speed;
    float spacing = 1.5;
    
    // Simplified transformations
    float rot = from.z*0.25 + from.y*0.5;
    pR(from.zy, rot); 
    pR(from.zx, -rot*0.1); 
    
    pR(from.yz, time*0.2);
    from.x = mod(from.x-time, spacing)-spacing*0.5;
    
    // Simplified radial modulation
    float angle = atan(from.y, from.x);
    float radius = VIEW_HEIGHT*(1.25+cos(time)*0.25);
    angle = mod(angle, PI*0.4) - PI*0.2;
    from.yz = vec2(cos(angle), sin(angle)) * radius;
    
    pR(from.zy, 1.45*time+angle);
    pR(from.yx, angle);
    
    // Use only one shape for better performance
    return fDodecahedron(from, (cos(time)+2.0)*0.67);
}

float castRay(in vec3 ro, in vec3 rd) {
    float t = 0.0;
    for(int i = 0; i < MAX_STEPS; i++){
        float res = map(ro + rd * t);
        if(abs(res) < EPSILON)
            return t;
        t += res;
        if(t > MAX_DIST)
            break;
    }
    return 1000.0;
}

vec3 getNormal(in vec3 from) {
    vec2 delta = vec2(0.02, 0.0); // Increased delta for better performance
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
    // Static camera - no motion for better performance
    // ro remains unchanged
}


vec3 render(in vec2 uv) {
    // Static camera for better performance
    vec3 ro = vec3(1.0, 0.000001, 0.0);
    vec3 lookAt = vec3(0.0, 0.0, 0.0);
    vec3 rd = getCam(ro, lookAt) * normalize(vec3(uv, FOV));

    // Scene geometry
    float dist = castRay(ro, rd);
    if(dist >= MAX_DIST) return vec3(0.0); // Early exit for background
    
    vec3 location = ro + rd * dist;
    vec3 world_norm = getNormal(location);
    vec3 cam_norm = normalize(world_norm * normalize(ro - lookAt));

    // Simplified lighting
    vec3 key_light = vec3(0.0, 3.4, 7.4);
    float key_exposure = dot(world_norm, normalize(key_light - location)) / log(10.0 * length(key_light - location));
    vec3 glint_light = vec3(0.0, 10.0, 0.0);
    vec3 fill_light = vec3(-10.0, 0.0, 0.0);

    // Simplified color calculations
    vec3 material_color = vec3(3.0, 0.4, 9.0);
    vec3 result_color = material_color * (min(vec3(key_exposure), material_color) * 0.5);
    result_color = max(result_color, material_color * pow(dot(world_norm, fill_light * 0.01) + 0.1, 2.8));
    result_color = max(pow(result_color * dot(cam_norm, glint_light) * 70000.0, vec3(1.9)), result_color);
    result_color = max(pow(result_color * dot(cam_norm, -glint_light) * 70000.0, vec3(1.9)), result_color);
    
    // Simplified fog
    return mix(result_color, vec3(0.0), 1.0 - exp(-0.0005 * dist * dist));
}


void mainImage(out vec4 fragColor, in vec2 fragCoord) {
    vec2 uv = (2.0 * fragCoord.xy - iResolution.xy) / iResolution.y;
    
    vec3 col = render(uv);
    
    // Simplified gamma correction
    col = pow(col * u_brightness, vec3(u_scale + 0.1));
    
    fragColor = vec4(col, 1.0);
}

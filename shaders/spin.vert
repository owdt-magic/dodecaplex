#include global.glsl

layout(location = 0) in vec4 model_verts;
layout(location = 1) in vec3 model_textures;
layout(location = 2) in vec4 model_normals;

#include projection.glsl

 uniform vec4 u_audio_bands;
 uniform float u_time;
 uniform float u_shatter;

vec4 addTextureToNormal(vec2 tex, vec4 normal) {
    // Step 1: normalize the input normal
    vec4 n = normalize(normal);

    // Step 2: build tangent basis
    // Pick a safe arbitrary vector not too aligned with n
    vec4 e1 = abs(n.x) < 0.9 ? vec4(1.0, 0.0, 0.0, 0.0) : vec4(0.0, 1.0, 0.0, 0.0);

    // First tangent
    vec4 u = normalize(e1 - dot(e1, n) * n);

    // Pick another candidate
    vec4 e2 = abs(n.z) < 0.9 ? vec4(0.0, 0.0, 1.0, 0.0) : vec4(0.0, 0.0, 0.0, 1.0);

    // Second tangent, orthogonalize against n and u
    vec4 v = normalize(e2 - dot(e2, n) * n - dot(e2, u) * u);

    // Step 3: embed tex coords into plane
    vec4 offset = tex.x * u + tex.y * v;

    // Step 4: add the texture-based offset to the normal
    return normalize(normal + offset);
}

void main(){
    mat4 supplemental_rotation = mat4(1.0);
    vec4 bands = u_audio_bands/200.0;
    supplemental_rotation *= mat4( 
        1.0,  0.0,    0.0, 0.0,
        0.0,  cos(bands.x), 0.0, sin(bands.x),
        0.0,  0.0,    1.0, 0.0,
        0.0, -sin(bands.x), 0.0, cos(bands.x)
    );
    supplemental_rotation *= mat4(
        cos(bands.y), 0.0, 0.0, sin(bands.y), 
        0.0,     1.0, 0.0, 0.0, 
        0.0,     0.0, 1.0, 0.0,     
       -sin(bands.y), 0.0, 0.0, cos(bands.y)
    );
    supplemental_rotation *= mat4(
        1.0, 0.0,  0.0,     0.0, 
        0.0, 1.0,  0.0,     0.0, 
        0.0, 0.0,  cos(bands.z), sin(bands.z), 
        0.0, 0.0, -sin(bands.z), cos(bands.z)
    );

    supplemental_rotation *= mat4(
        1.0,  0.0,     0.0,     0.0,
        0.0,  cos(bands.y), sin(bands.y), 0.0,
        0.0, -sin(bands.y), cos(bands.y), 0.0,
        0.0,  0.0,     0.0,     1.0
    );    
    supplemental_rotation *= mat4(
        cos(bands.z), 0.0, -sin(bands.z), 0.0,
        0.0,     1.0,  0.0,     0.0,
        sin(bands.z), 0.0,  cos(bands.z), 0.0,
        0.0,     0.0,  0.0,     1.0
    );
    supplemental_rotation *= mat4(
        cos(bands.w), sin(bands.w), 0.0, 0.0,
       -sin(bands.w), cos(bands.w), 0.0, 0.0,
        0.0,     0.0,     1.0, 0.0,
        0.0,     0.0,     0.0, 1.0
    );
    vec4 offset = addTextureToNormal(model_textures.xy*100000.0, model_normals);
    processVerts(supplemental_rotation*(model_verts + offset*u_shatter/200.0));
}
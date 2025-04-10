
out vec4 mCoords;
out vec4 wCoords;
out vec3 tCoords;

#define FOCUS sqrt(5)

void project(inout vec4 vert){
    vert.xyz *= FOCUS/(FOCUS + vert.w);
    vert.w = 1.0;
}

void processVerts(vec4 verts) {
    verts = WORLD * verts;
    mCoords = verts;
    project(verts);
    wCoords = verts;
    tCoords = model_textures;
    gl_Position = CAMERA * verts;
}
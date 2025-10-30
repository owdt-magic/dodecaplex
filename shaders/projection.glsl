
out vec4 mCoords;
out vec4 wCoords;
out vec3 tCoords;

#define FOCUS sqrt(5)

void project(inout vec4 vert){
    vert.xyz *= FOCUS/(FOCUS + vert.w);
    vert.w = 1.0;
}

void processVerts(vec4 verts) {
    // Check for background flag: w = -999.0 means skip transformations
    if (verts.w < -500.0) {
        // Background geometry: pass clip-space coordinates directly
        // Create varying model coords for spatial spell effects
        float scale = 50.0;
        mCoords = vec4(verts.x * scale, verts.y * scale, -scale, 1.0);
        wCoords = vec4(verts.x * scale, verts.y * scale, 0.0, 1.0);
        tCoords = model_textures;
        gl_Position = vec4(verts.xyz, 1.0);  // Pass through clip-space coords
        return;
    }

    // Regular 4D geometry: apply standard transformations
    verts = WORLD * verts;
    mCoords = verts;
    project(verts);
    wCoords = verts;
    tCoords = model_textures;
    gl_Position = CAMERA * verts;
}
#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 mCoords[];
in vec4 wCoords[];
in vec3 tCoords[];

out vec4 model_Coords;
out vec3 texture_Coords;
out float zDepth;


bool isBackgroundTriangle() {
    // Background triangles are marked with texture_Coords.r = 1.0 (red)
    // Check if ALL vertices are marked (more robust than any)
    return (abs(tCoords[0].r) > 0.9 && abs(tCoords[1].r) > 0.9 && abs(tCoords[2].r) > 0.9);
}

bool goodTriangle() {
    float leg;
    float longest = -1.0;
    float max_offset =  max(length(wCoords[0].xyz),
                        max(length(wCoords[1].xyz),
                            length(wCoords[2].xyz)));
    vec3 arc_tans;

    for (int i = 0; i < 3; i++) {
        arc_tans[i] = atan(mCoords[i].z, mCoords[i].w);
        leg = length(wCoords[i].xyz-wCoords[(i+1)%3].xyz);
        if (max_offset < leg) {
            // aka - does this triangle 'wrap' the origin?
            longest = max(longest, leg);
        }
    }

    return (
        longest    < 1.0 &&
        arc_tans.x < 0.5 &&
        arc_tans.y < 0.5 &&
        arc_tans.z < 0.5
    );
}

void main() {

    if (isBackgroundTriangle() || goodTriangle()) {
        for(int i = 0; i < 3; i++) {
            gl_Position     = gl_in[i].gl_Position;
            model_Coords    = mCoords[i];
            texture_Coords  = tCoords[i];
            zDepth          = gl_Position.z;
            EmitVertex();
        }
        EndPrimitive();
    }
}
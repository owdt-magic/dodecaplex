#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in vec4 mCoords[];
in vec4 wCoords[];
in vec3 tCoords[];
uniform vec2 u_resolution;

out vec4 model_Coords;
out vec3 texture_Coords;
out vec3 bary_Coords;
out float zDepth;

flat out vec2 wP0, wP1, wP2;

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
    if (goodTriangle()) {
        vec4 c0 = gl_in[0].gl_Position;
        vec4 c1 = gl_in[1].gl_Position;
        vec4 c2 = gl_in[2].gl_Position;

        vec2 ndc0 = (c0.xy / c0.w) * 0.5 + 0.5;
        vec2 ndc1 = (c1.xy / c1.w) * 0.5 + 0.5;
        vec2 ndc2 = (c2.xy / c2.w) * 0.5 + 0.5;

        wP0 = ndc0 * u_resolution;
        wP1 = ndc1 * u_resolution;
        wP2 = ndc2 * u_resolution;

        for(int i = 0; i < 3; i++) {
            vec3 bary_coords = vec3(0.0);
            bary_coords[i]   = 1.0;

            gl_Position     = gl_in[i].gl_Position;            
            model_Coords    = mCoords[i];
            texture_Coords  = tCoords[i];       
            bary_Coords     = bary_coords;
            zDepth          = gl_Position.z;
            EmitVertex();
        }
        EndPrimitive();
    }
}
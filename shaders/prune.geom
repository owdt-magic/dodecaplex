#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in float vDepth[];
in vec4 mCoords[];
in vec4 wCoords[];

out float zDepth;
out vec4 model_Coords;
out vec3 texture_Coords;

#define THRESH 10.0

void main() {
    float leg;
    float longest = -1.0;
    float max_offset =  max(length(wCoords[0].xyz), 
                        max(length(wCoords[1].xyz), 
                            length(wCoords[2].xyz)));
    vec3 corners[5];
    vec3 sides;
    int texture_indexes[3];
    vec3 arc_tans;
    
    for (int i = 0; i < 3; i++) {
        arc_tans[i] = atan(mCoords[i].z, mCoords[i].w);
        leg = length(wCoords[i].xyz-wCoords[(i+1)%3].xyz);
        if (max_offset < leg) {
            // aka - does this triangle 'wrap' the origin?
            longest = max(longest, leg);
        }
    }
    sides = vec3(   length(mCoords[0]-mCoords[1]), 
                    length(mCoords[1]-mCoords[2]), 
                    length(mCoords[2]-mCoords[0]));
    sides/=max(sides.x, max(sides.y, sides.z));
    corners[0] = vec3(0.48,0.0,2.); 
    corners[1] = vec3(0.0,0.38,2.); 
    corners[2] = vec3(0.2,0.95,2.);
    corners[3] = vec3(0.8,0.95,2.);
    corners[4] = vec3(1.0,0.38,2.);
    
    if (longest < THRESH  &&
        arc_tans[0] < 0.5 &&
        arc_tans[1] < 0.5 &&
        arc_tans[2] < 0.5
    ) {
        if ((sides.x+sides.y > 1.8) ||
            (sides.y+sides.z > 1.8) ||
            (sides.z+sides.x > 1.8)) {
                texture_indexes[0] = 0;
                texture_indexes[1] = 2;
                texture_indexes[2] = 3;
            }
        else if((sides.x > 0.9)) {
            texture_indexes[0] = 0;
            texture_indexes[1] = 2;
            texture_indexes[2] = 1;            
        } else {
            texture_indexes[0] = 0;
            texture_indexes[1] = 4;
            texture_indexes[2] = 3;
        }
        for(int i = 0; i < 3; i++) {
            gl_Position     = gl_in[i].gl_Position;
            zDepth          = vDepth[i];
            model_Coords    = mCoords[i];
            texture_Coords  = corners[texture_indexes[i]];
            EmitVertex();
        }
        EndPrimitive();
    }
}
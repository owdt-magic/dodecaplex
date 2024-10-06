#version 410 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in float vDepth[];
in vec4 mCoords[];
in vec4 wCoords[];

out float zDepth;
out vec4 model_Coords;

#define THRESH 10.0

void main() {
    float leg;
    float longest = -1.0;
    float max_offset =  max(length(wCoords[0].xyz), 
                        max(length(wCoords[1].xyz), 
                            length(wCoords[2].xyz)));

    for (int i = 0; i < 3; i++) {
        leg = length(wCoords[i].xyz-wCoords[(i+1)%3].xyz);
        if (max_offset < leg) {
            // aka - does this triangle 'wrap' the origin?
            longest = max(longest, leg);
        }
    }
    if (longest < THRESH) {
        for(int i = 0; i < 3; i++) {
            gl_Position     = gl_in[i].gl_Position;
            zDepth          = vDepth[i];
            model_Coords    = mCoords[i];
            EmitVertex();
        }
        EndPrimitive();
    }
}
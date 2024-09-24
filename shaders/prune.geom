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
    float point1, point2;
    float longest = -1.0;
    for (int i = 0; i < 3; i++) {
        point1 = length(wCoords[i].xyz);
        point2 = length(wCoords[(i+1)%3].xyz);
        leg = length(wCoords[i].xyz-wCoords[(i+1)%3].xyz);
        if (max(point1, point2) < leg){
            // aka - does this triangle 'wrap' the origin?
            longest = max(longest, leg);
        }
    }
    if (longest < THRESH) {
        for(int i = 0; i < 3; i++) {
            gl_Position = gl_in[i].gl_Position;
            zDepth = vDepth[i];
            model_Coords = mCoords[i];
            EmitVertex();
        }
        EndPrimitive();
    }
}
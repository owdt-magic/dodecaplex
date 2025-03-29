#ifndef PENTAGON_H
#define PENTAGON_H

#include "dmath.h"
#include "dodecaplex.h"
#include "bufferObjects.h"
#include <array>

struct PentagonMemory {
    int v_start, v_end, i_start, i_end, i_offset, source;
    int v_len, i_len;
    
    std::array<glm::vec4, 5> corners;
    std::array<glm::vec4, 2> centroids;
    std::array<glm::vec4, 5> centered;
    
    glm::mat4 rotation;
    glm::vec4 offset;
    glm::vec4 normal;

    PentagonMemory() {};
    PentagonMemory(int src);
    glm::mat4 solveRotation(std::array<glm::vec4, 5> start);
    void markStart(CPUBufferPair& bw);
    void markEnd(CPUBufferPair& bw);
};

#endif
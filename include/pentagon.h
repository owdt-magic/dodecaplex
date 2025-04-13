#ifndef PENTAGON_H
#define PENTAGON_H

#include "dmath.h"
#include "dodecaplex.h"
#include "bufferObjects.h"
#include <array>

enum Surface 
{
    ORIGINAL,
    BROKEN
};

struct PentagonMemory {
    int v_start, v_end, i_start, i_end, i_offset, source;
    int v_len, i_len;
    
    std::array<glm::vec4, 5> corners;
    std::array<glm::vec4, 2> centroids;
    std::array<glm::vec4, 5> centered;
    
    glm::mat4 rotation;
    glm::vec4 offset;
    glm::vec4 normal;
    bool has_rotation = false;
    Surface surface = Surface::ORIGINAL;
    
    PentagonMemory() {};
    PentagonMemory(int src);
    glm::mat4 solveRotation(std::array<glm::vec4, 5> start, bool force);
    void markStart(CPUBufferPair& bw);
    void markEnd(CPUBufferPair& bw);

    std::array<std::pair<PentagonMemory*, bool>, 5> neighbors;
    void addNeighbor(PentagonMemory* other, bool in_out);
};

#endif
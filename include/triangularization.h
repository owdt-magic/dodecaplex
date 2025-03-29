#ifndef TRIANGULARIZATION_H
#define TRIANGULARIZATION_H

#include "pentagon.h"
#include <vector>
#include <utility>

enum RhombusType {
    WIDE,
    THIN
};

enum Corner {
    BOTTOM=0,
    LEFT=1,
    TOP=2,
    RIGHT=3
};

enum SplitType {
    LONG,
    SHORT
};

enum SkipType {
    NONE,
    FIRST,
    SECOND
};

enum WebType {
    SIMPLE_STAR
};

struct RhombusIndeces {
    GLuint triangle_a[3];
    GLuint triangle_b[3];
};

struct GoldenRhombus {
    GoldenRhombus();
    GoldenRhombus(RhombusType type, Corner origin, uint& offset);
        // Initiallizes origin rhombus, aligned with the x axis
    GoldenRhombus(GoldenRhombus& neighbor, RhombusType type, 
                    std::pair<Corner, Corner> corner_1, 
                    std::pair<Corner, Corner> corner_2, uint& offset);
        // Uses the corner pair enums to map 2 corners from neighbor to 2 of its own corners.
    GoldenRhombus(GoldenRhombus& neighbor_a, GoldenRhombus& neighbor_b, RhombusType type, 
                    std::pair<Corner, Corner> corner_a1, 
                    std::pair<Corner, Corner> corner_a2, 
                    std::pair<Corner, Corner> corner_b, uint& offset);
        // Uses the first enums 2 for neighbor_a, and the third from b,
        // NOTE: Always use this initialization when possible!
        // NOTE: There is no 4 corner version - build the RhombusWeb in an order that accounts!
    void writeUints(GLuint* start, int& head, uint i_offset);
    void writeFloats(GLfloat* start, int& head, PentagonMemory& pentagon, float texture);
    glm::vec3 corners[4]; // always clockwise!!
    enum SplitType split = SplitType::SHORT;
    enum SkipType skip = SkipType::NONE;
private:
    RhombusIndeces getIndeces();
    void shareCorner(GoldenRhombus& source, std::pair<Corner, Corner> corner);
    bool uniques[4] = {true, true, true, true}; // Avoid redundant counting
    uint indeces[4];
    GoldenRhombus* branches[4];
};

struct RhombusWeb {
    std::vector<GoldenRhombus> all_rhombuses;
    float pentagon_scale;
    float vertical_offset;
    uint offset;
    float web_texture = 1.0f;
    RhombusWeb(WebType pattern, bool flip);
    void buildArrays(CPUBufferPair& buffer_writer, PentagonMemory& pentagon);
    std::array<glm::vec4,5> web_pentagon;
private:
    void assignCorners(std::array<GoldenRhombus, 5>& rhombuses, Corner corner);
    void rescaleValues();
};

#endif
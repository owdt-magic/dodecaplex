#ifndef RHOMBUS_H
#define RHOMBUS_H

#include "pentagon.h"
#include <vector>
#include <utility>
#include <map>

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
    VERT,
    HORZ
};

enum SkipType {
    NONE,
    FIRST,
    SECOND,
    BOTH
};

enum WebType {
    SIMPLE_STAR,
    DOUBLE_STAR
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
        // NOTE: There is no 4 corner version - build the RhombusPattern in an order that accounts!
    void writeUints(GLuint* start, int& head, uint i_offset);
    void writeFloats(GLfloat* start, int& head, PentagonMemory& pentagon, float texture, 
        bool flip_norms, bool flip_text, bool write_norms);
    glm::vec3 corners[4]; // always clockwise!!
    uint indeces[4];
    std::vector<std::pair<GoldenRhombus*, Corner>> shared[4];
    enum SplitType split = SplitType::HORZ;
    enum SkipType skip = SkipType::NONE;
    bool uniques[4] = {true, true, true, true}; // Avoid redundant counting
    glm::vec4 getTransformedCorner(enum Corner corner, const PentagonMemory& pentagon, bool flip_norms);
    void printUints();
    void printFloats();
private:
    RhombusIndeces getIndeces();
    void shareCorner(GoldenRhombus& source, std::pair<Corner, Corner> corner);
    void shareSide(GoldenRhombus& neighbor,
                        std::pair<Corner, Corner> corner_1,
                        std::pair<Corner, Corner> corner_2);
    GoldenRhombus* branches[4] = {NULL};
};

struct VertexRankResult {
    int web_index;
    float radius;
    GoldenRhombus* source;
    Corner corner;
    VertexRankResult(int wi, float r, GoldenRhombus* gr, Corner c) : 
        web_index(wi), radius(r), source(gr), corner(c) {};
};

struct RhombusPattern {
    float pentagon_scale;
    float vertical_offset;
    uint offset;
    int vertex_count = 0;
    int index_count = 0;
    bool flipped;
    bool upsidedown = false;
    float web_texture = 1.0f;
    RhombusPattern(WebType pattern, bool flip);
    void buildArrays(CPUBufferPair& buffer_writer, PentagonMemory& pentagon, bool include_normals);
    void buildArrays(CPUBufferPair& buffer_writer, PentagonMemory& pentagon);
    std::array<glm::vec4,5> web_pentagon;
    std::array<std::pair<GoldenRhombus*, Corner>, 5> corners;
    void applyDamage(CPUBufferPair& buffer_writer, glm::mat4 player_view, PentagonMemory& pentagon);
    void applyFootprints(CPUBufferPair& buffer_writer, glm::mat4 player_view, PentagonMemory& pentagon);
    std::map<uint,std::pair<int,int>> edge_map;
private:
    uint num_verts = 0;
    std::vector<GoldenRhombus> all_rhombuses;
    std::vector<VertexRankResult> ranked_verts;
    std::vector<GoldenRhombus*> edges[5];
    void pushAndCount(GoldenRhombus rhombus);
    template<long unsigned int N>
    void addRhombuses(std::array<GoldenRhombus, N>& rhombuses);
    template<long unsigned int N>
    void addRhombuses(std::array<GoldenRhombus, N>& rhombuses, SkipType skip);
    template<long unsigned int N>
    void addRhombuses(std::array<GoldenRhombus, N>& rhombuses, SkipType skip, SplitType split);
    template<long unsigned int N>
    void addRhombuses(std::array<GoldenRhombus, N>& rhombuses, SplitType split);
    void assignCorners(std::array<GoldenRhombus, 5>& rhombuses, Corner corner);
    void assignCorners(std::array<GoldenRhombus*, 5> rhombuses, Corner corner);
    template<long unsigned int N>
    void assignEdge(std::array<GoldenRhombus*, N> rhombuses, int edge_index);
    void rescaleValues();
    void countVerts();
    void rankVerts(glm::mat4& player_view, PentagonMemory& pentagon);
    void writeObj();
    template<typename BufferOperation>
    void overwriteBuffer(CPUBufferPair& buffer_writer, glm::mat4 player_view, PentagonMemory& pentagon, BufferOperation operation);

};

#endif
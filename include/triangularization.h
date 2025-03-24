#include "dmath.h"
#include <vector>
#include <utility>
#include <array>

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
    void writeFloats(GLfloat* start, int& head, float in_scale, float out_scale,
                        glm::mat4& rotation_mat, glm::vec4& in_offset, glm::vec4& out_offset);
    glm::vec3 readCorner(Corner corner);
private:
    RhombusIndeces getIndeces();
    void shareCorner(GoldenRhombus& source, std::pair<Corner, Corner> corner);
    glm::vec3 corners[4]; // always clockwise!!
    bool uniques[4] = {true, true, true, true}; // Avoid redundant counting
    enum SplitType split = SplitType::SHORT;
    enum SkipType skip = SkipType::NONE;
    uint indeces[4];
    GoldenRhombus* branches[4];
};

struct RhombusWeb {
    std::vector<GoldenRhombus> collection;
    float scale;
    float centroid_offset;
    uint offset;
    RhombusWeb();
    void fillCorners(std::array<glm::vec4, 5> &dest);
    void buildArrays(GLfloat* v_buffer, GLuint* i_buffer, int& v_head, int& i_head, uint i_offset,
                        std::array<glm::vec4, 5> dest_pentagon, std::array<glm::vec4, 2> dest_centroids);
    std::array<glm::vec3,5> bounds;
};
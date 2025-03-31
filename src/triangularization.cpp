#include "triangularization.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>

using namespace glm;

using std::pair;
using std::make_pair;
using std::array;

#define PI 3.1415f
#define AY tan(0.3f*PI)
#define AZ sqrt(PHI + 1.0f - AY*AY)

#define BY PHI / tan(0.4f*PI)
#define BZ sqrt(1.0f - BY*BY)

#define COSTPF cos(2.0f*PI/5.0f)
#define SINTPF sin(2.0f*PI/5.0f)
#define COSNTPF cos(-2.0f*PI/5.0f)
#define SINNTPF sin(-2.0f*PI/5.0f)
#define COSPF cos(PI/5.0f)
#define SINPF sin(PI/5.0f)
#define DODECAPLEX_SIDE_LEN (3.0f-sqrt(5.0f))
#define CIRCUMRADIUS_RATIO sqrt((5.0f+sqrt(5.0f))/10.0f)

float Rx_WIDE(vec2 xy) {
    return COSTPF*xy.x - SINTPF*xy.y;
};
float Ry_WIDE(vec2 xy) {
    return SINTPF*xy.x + COSTPF*xy.y;
};
float Rx_WIDE_R(vec2 xy) {
    return COSNTPF*xy.x - SINNTPF*xy.y;
};
float Ry_WIDE_R(vec2 xy) {
    return SINNTPF*xy.x + COSNTPF*xy.y;
};
float Rx_THIN(vec2 xy) {
    return COSPF*xy.x - SINPF*xy.y;
};
float Ry_THIN(vec2 xy) {
    return SINPF*xy.x + COSPF*xy.y;
};

GoldenRhombus::GoldenRhombus(){};

GoldenRhombus::GoldenRhombus(RhombusType type, Corner origin, uint& offset){
    corners[origin] = vec3(0.);
    switch (type) 
    {
    case RhombusType::WIDE:
        switch (origin) 
        {
        case Corner::BOTTOM:
            corners[Corner::LEFT]  = vec3( 1.0f, AY, AZ)*0.5f;
            corners[Corner::TOP]   = vec3( 0.0f, AY, AZ);
            corners[Corner::RIGHT] = vec3(-1.0f, AY, AZ)*0.5f;
            break;
        case Corner::LEFT:
            throw std::invalid_argument("Origin WIDE rhombus must use Corner::TOP/Corner::BOTTOM");
        case Corner::TOP:
            corners[Corner::BOTTOM] = vec3( 0.0f, AY, -AZ);
            corners[Corner::LEFT]   = vec3(-1.0f, AY, -AZ)*0.5f;
            corners[Corner::RIGHT]  = vec3( 1.0f, AY, -AZ)*0.5f;
            break;
        case Corner::RIGHT:
            throw std::invalid_argument("Origin WIDE rhombus must use Corner::TOP/Corner::BOTTOM");
        }
        break;
    case RhombusType::THIN:
        switch (origin) 
        {
        case Corner::BOTTOM:
            throw std::invalid_argument("Origin THIN rhombus must use Corner::LEFT/Corner::RIGHT");
        case Corner::LEFT:
            corners[Corner::BOTTOM] = vec3(   BY, PHI, -BZ)*0.5f;
            corners[Corner::TOP]    = vec3(  -BY, PHI,  BZ)*0.5f;
            corners[Corner::RIGHT]  = vec3( 0.0f, PHI, 0.0f);
            break;
        case Corner::TOP:
            throw std::invalid_argument("Origin THIN rhombus must use Corner::LEFT/Corner::RIGHT");
        case Corner::RIGHT:
            corners[Corner::BOTTOM] = vec3(  -BY, PHI,  BZ)*0.5f;
            corners[Corner::LEFT]   = vec3( 0.0f, PHI, 0.0f);
            corners[Corner::TOP]    = vec3(   BY, PHI, -BZ)*0.5f;            
            break;
        }
        break;
    }
    indeces[offset++] = Corner::BOTTOM;
    indeces[offset++] = Corner::LEFT;
    indeces[offset++] = Corner::TOP; 
    indeces[offset++] = Corner::RIGHT;
}

void GoldenRhombus::shareCorner(GoldenRhombus& source, pair<Corner, Corner> corner) {
    corners[corner.second] = source.corners[corner.first];
    indeces[corner.second] = source.indeces[corner.first];
    uniques[corner.second] = false;

    source.branches[corner.first] = this;
}

GoldenRhombus::GoldenRhombus(GoldenRhombus& neighbor, RhombusType type, 
                                pair<Corner, Corner> corner_1,
                                pair<Corner, Corner> corner_2, uint& offset){
    vec2 rot;
    vec3 change;
    shareCorner(neighbor, corner_1);
    shareCorner(neighbor, corner_2);
    change = corners[corner_1.second] - corners[corner_2.second];

    switch (corner_1.second) 
    {
    case Corner::TOP:
        switch (corner_2.second) 
        {
        case Corner::LEFT: throw std::invalid_argument("Not implemented");
        case Corner::RIGHT:
            rot.x = corners[Corner::RIGHT].x - corners[Corner::TOP].x;
            rot.y = corners[Corner::RIGHT].y - corners[Corner::TOP].y;

            corners[Corner::LEFT]   = vec3( Rx_WIDE(rot) + corners[Corner::TOP].x,
                                            Ry_WIDE(rot) + corners[Corner::TOP].y, 
                                                           corners[Corner::RIGHT].z);
            corners[Corner::BOTTOM] = corners[Corner::LEFT] - change;

            indeces[Corner::BOTTOM] = offset++;
            indeces[Corner::LEFT]   = offset++;
            
            break;
        default: throw std::invalid_argument("Specify horizontal destination corner with corner_2 variable");
        }
        break;
    case Corner::BOTTOM:
        switch (corner_2.second) 
        {        
        case Corner::LEFT: 
            rot.x = corners[Corner::LEFT].x - corners[Corner::BOTTOM].x;
            rot.y = corners[Corner::LEFT].y - corners[Corner::BOTTOM].y;
            
            corners[Corner::RIGHT] = vec3( Rx_WIDE(rot) + corners[Corner::BOTTOM].x,
                                           Ry_WIDE(rot) + corners[Corner::BOTTOM].y, 
                                                          corners[Corner::LEFT].z);
            corners[Corner::TOP]   = corners[Corner::RIGHT] - change;

            indeces[Corner::TOP]   = offset++;
            indeces[Corner::RIGHT] = offset++;

            break;
        case Corner::RIGHT: 
            rot.x = corners[Corner::RIGHT].x - corners[Corner::BOTTOM].x;
            rot.y = corners[Corner::RIGHT].y - corners[Corner::BOTTOM].y;
            
            corners[Corner::LEFT] = vec3( Rx_WIDE_R(rot) + corners[Corner::BOTTOM].x,
                                          Ry_WIDE_R(rot) + corners[Corner::BOTTOM].y, 
                                                           corners[Corner::RIGHT].z);
            corners[Corner::TOP]  = corners[Corner::LEFT] - change;

            indeces[Corner::LEFT] = offset++;
            indeces[Corner::TOP]  = offset++;
            
            break;
        default: throw std::invalid_argument("Specify horizontal destination corner with corner_2 variable");
        }
        break;
    default: throw std::invalid_argument("Specify vertical destination corner with corner_1 variable");
    }
}

GoldenRhombus::GoldenRhombus(GoldenRhombus& neighbor_a, GoldenRhombus& neighbor_b, RhombusType type, 
                                pair<Corner, Corner> corner_a1, 
                                pair<Corner, Corner> corner_a2, 
                                pair<Corner, Corner> corner_b, uint& offset){
    vec2 rot;
    vec3 change;
    
    shareCorner(neighbor_a, corner_a1);
    shareCorner(neighbor_a, corner_a2);
    shareCorner(neighbor_b, corner_b);
    
    change = corners[corner_a1.second] - corners[corner_a2.second];

    switch (corner_a1.second) 
    {
    case Corner::TOP:
        switch (corner_a2.second) 
        {
        case Corner::LEFT: throw std::invalid_argument("Not implemented");            
        case Corner::RIGHT:
            switch (corner_b.second) 
            {
            case Corner::LEFT:
                corners[Corner::BOTTOM] = corners[Corner::LEFT] - change;
                indeces[Corner::BOTTOM] = offset++;
                break;
            case Corner::BOTTOM:
                corners[Corner::LEFT] = corners[Corner::BOTTOM] + change;
                indeces[Corner::LEFT] = offset++;
                break;
            default:
                throw std::invalid_argument("Not implemented");
            }
            break;
        default: throw std::invalid_argument("Specify horizontal destination corner with corner_2 variable");
        }
        break;
    case Corner::BOTTOM: 
        switch (corner_a2.second) 
        {
        case Corner::RIGHT: 
            switch (corner_b.second) 
            {
            case Corner::LEFT:
                corners[Corner::TOP] = corners[Corner::LEFT] - change;
                indeces[Corner::TOP] = offset++;
                break;
            default:
                throw std::invalid_argument("Not implemented");
            }
            break;
        case Corner::LEFT:
            switch (corner_b.second) 
            {
            case Corner::RIGHT:
                corners[Corner::TOP] = corners[Corner::RIGHT] - change;
                indeces[Corner::TOP] = offset++;
                break;
            default:
                throw std::invalid_argument("Not implemented");
            }
            break;
        default: throw std::invalid_argument("Specify horizontal destination corner with corner_2 variable");
        }
        break;        
    default: throw std::invalid_argument("Specify vertical destination corner with corner_1 variable");
    }

}

void RhombusWeb::assignCorners(array<GoldenRhombus, 5>& rhombuses, Corner corner) {
    // If this is used correctly, these two values should be identical for every corner...
    pentagon_scale  = length(vec2(rhombuses[0].corners[corner]));
    vertical_offset = rhombuses[0].corners[corner].z;
    
    for (int i=0; i < 5; i++) web_pentagon[i] = vec4(vec2(rhombuses[i].corners[corner]), 0.0f, 0.0f);
    rescaleValues();
}

void RhombusWeb::rescaleValues(){
    for (GoldenRhombus& rhombus : all_rhombuses) {
        for (vec3& corner : rhombus.corners) {
            corner.z -= vertical_offset;
            corner *= ( (DODECAPLEX_SIDE_LEN * CIRCUMRADIUS_RATIO) / pentagon_scale );
        }
    }
}

RhombusWeb::RhombusWeb(WebType pattern, bool flip) : flipped(flip) {
    offset = 0;
    array<GoldenRhombus, 5> center;
    array<GoldenRhombus, 5> edges;

    switch (pattern)
    {
    case WebType::DOUBLE_STAR:
    case WebType::SIMPLE_STAR:
        center[0] = GoldenRhombus(RhombusType::WIDE, Corner::TOP, offset);
        center[1] = GoldenRhombus(center[0], RhombusType::WIDE,
                        make_pair(Corner::TOP, Corner::TOP), 
                        make_pair(Corner::LEFT, Corner::RIGHT), offset);
        center[2] = GoldenRhombus(center[1], RhombusType::WIDE,
                        make_pair(Corner::TOP, Corner::TOP), 
                        make_pair(Corner::LEFT, Corner::RIGHT), offset);
        center[3] = GoldenRhombus(center[2], RhombusType::WIDE,
                        make_pair(Corner::TOP, Corner::TOP), 
                        make_pair(Corner::LEFT, Corner::RIGHT), offset);
        center[4] = GoldenRhombus(center[3], center[0], RhombusType::WIDE,
                        make_pair(Corner::TOP, Corner::TOP), 
                        make_pair(Corner::LEFT, Corner::RIGHT), 
                        make_pair(Corner::RIGHT, Corner::LEFT), offset);
    
        edges[0] = GoldenRhombus(center[0], center[1], RhombusType::THIN,
                        make_pair(Corner::LEFT, Corner::TOP), 
                        make_pair(Corner::BOTTOM, Corner::RIGHT), 
                        make_pair(Corner::BOTTOM, Corner::LEFT), offset);
        edges[1] = GoldenRhombus(center[1], center[2], RhombusType::THIN,
                        make_pair(Corner::LEFT, Corner::TOP), 
                        make_pair(Corner::BOTTOM, Corner::RIGHT), 
                        make_pair(Corner::BOTTOM, Corner::LEFT), offset);
        edges[2] = GoldenRhombus(center[2], center[3], RhombusType::THIN,
                        make_pair(Corner::LEFT, Corner::TOP), 
                        make_pair(Corner::BOTTOM, Corner::RIGHT), 
                        make_pair(Corner::BOTTOM, Corner::LEFT), offset);
        edges[3] = GoldenRhombus(center[3], center[4], RhombusType::THIN,
                        make_pair(Corner::LEFT, Corner::TOP), 
                        make_pair(Corner::BOTTOM, Corner::RIGHT), 
                        make_pair(Corner::BOTTOM, Corner::LEFT), offset);
        edges[4] = GoldenRhombus(center[4], center[0], RhombusType::THIN,
                        make_pair(Corner::LEFT, Corner::TOP), 
                        make_pair(Corner::BOTTOM, Corner::RIGHT), 
                        make_pair(Corner::BOTTOM, Corner::LEFT), offset);
        break;
    }
    switch (pattern)
    {
    case WebType::SIMPLE_STAR:
        all_rhombuses.reserve(10);
        for (GoldenRhombus r : center) all_rhombuses.push_back(r);
        for (GoldenRhombus r : edges) {
            r.skip = SkipType::FIRST;
            all_rhombuses.push_back(r);
        }
        assignCorners(center, Corner::BOTTOM);
        return;
    case WebType::DOUBLE_STAR:
        array<GoldenRhombus, 15> wide_loop;
        array<GoldenRhombus, 10> thin_edge;
        array<GoldenRhombus, 5> corners;

        wide_loop[0] = GoldenRhombus(edges[0], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM),
                        make_pair(Corner::LEFT, Corner::RIGHT), offset);
        wide_loop[1] = GoldenRhombus(wide_loop[0], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM),
                        make_pair(Corner::LEFT, Corner::RIGHT), offset);
        wide_loop[2] = GoldenRhombus(wide_loop[1], edges[0], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM),
                        make_pair(Corner::LEFT, Corner::RIGHT),
                        make_pair(Corner::RIGHT, Corner::LEFT), offset);
        wide_loop[3] = GoldenRhombus(wide_loop[2], edges[4], RhombusType::WIDE,
                        make_pair(Corner::TOP, Corner::TOP),
                        make_pair(Corner::LEFT, Corner::RIGHT),
                        make_pair(Corner::BOTTOM, Corner::BOTTOM), offset);
        wide_loop[4] = GoldenRhombus(wide_loop[3], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM),
                        make_pair(Corner::LEFT, Corner::RIGHT), offset);
        wide_loop[5] = GoldenRhombus(wide_loop[4], edges[4], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM),
                        make_pair(Corner::LEFT, Corner::RIGHT),
                        make_pair(Corner::RIGHT, Corner::LEFT), offset);
        wide_loop[6] = GoldenRhombus(wide_loop[5], edges[3], RhombusType::WIDE,
                        make_pair(Corner::TOP, Corner::TOP),
                        make_pair(Corner::LEFT, Corner::RIGHT),
                        make_pair(Corner::BOTTOM, Corner::BOTTOM), offset);
        wide_loop[7] = GoldenRhombus(wide_loop[6], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM),
                        make_pair(Corner::LEFT, Corner::RIGHT), offset);
        wide_loop[8] = GoldenRhombus(wide_loop[7], edges[3], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM),
                        make_pair(Corner::LEFT, Corner::RIGHT),
                        make_pair(Corner::RIGHT, Corner::LEFT), offset);
        wide_loop[9] = GoldenRhombus(wide_loop[8], edges[2], RhombusType::WIDE,
                        make_pair(Corner::TOP, Corner::TOP),
                        make_pair(Corner::LEFT, Corner::RIGHT),
                        make_pair(Corner::BOTTOM, Corner::BOTTOM), offset);
        wide_loop[10] = GoldenRhombus(wide_loop[9], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM),
                        make_pair(Corner::LEFT, Corner::RIGHT), offset);
        wide_loop[11] = GoldenRhombus(wide_loop[10], edges[2], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM),
                        make_pair(Corner::LEFT, Corner::RIGHT),
                        make_pair(Corner::RIGHT, Corner::LEFT), offset);
        wide_loop[12] = GoldenRhombus(wide_loop[11], edges[1], RhombusType::WIDE,
                        make_pair(Corner::TOP, Corner::TOP),
                        make_pair(Corner::LEFT, Corner::RIGHT),
                        make_pair(Corner::BOTTOM, Corner::BOTTOM), offset);
        wide_loop[13] = GoldenRhombus(wide_loop[12], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM), 
                        make_pair(Corner::LEFT, Corner::RIGHT), offset);
        wide_loop[14] = GoldenRhombus(wide_loop[13], edges[1], RhombusType::WIDE,
                        make_pair(Corner::BOTTOM, Corner::BOTTOM),
                        make_pair(Corner::LEFT, Corner::RIGHT),
                        make_pair(Corner::RIGHT, Corner::LEFT), offset);
        
        thin_edge[0] = GoldenRhombus(wide_loop[0], wide_loop[1], RhombusType::WIDE,
                        make_pair(Corner::LEFT, Corner::BOTTOM),
                        make_pair(Corner::TOP, Corner::RIGHT),
                        make_pair(Corner::TOP, Corner::LEFT), offset);
        thin_edge[1] = GoldenRhombus(wide_loop[1], wide_loop[2], RhombusType::WIDE,
                        make_pair(Corner::LEFT, Corner::BOTTOM),
                        make_pair(Corner::TOP, Corner::RIGHT),
                        make_pair(Corner::TOP, Corner::LEFT), offset);
        thin_edge[2] = GoldenRhombus(wide_loop[3], wide_loop[4], RhombusType::WIDE,
                        make_pair(Corner::LEFT, Corner::BOTTOM),
                        make_pair(Corner::TOP, Corner::RIGHT),
                        make_pair(Corner::TOP, Corner::LEFT), offset);
        thin_edge[3] = GoldenRhombus(wide_loop[4], wide_loop[5], RhombusType::WIDE,
                        make_pair(Corner::LEFT, Corner::BOTTOM),
                        make_pair(Corner::TOP, Corner::RIGHT),
                        make_pair(Corner::TOP, Corner::LEFT), offset);
        thin_edge[4] = GoldenRhombus(wide_loop[6], wide_loop[7], RhombusType::WIDE,
                        make_pair(Corner::LEFT, Corner::BOTTOM),
                        make_pair(Corner::TOP, Corner::RIGHT),
                        make_pair(Corner::TOP, Corner::LEFT), offset);
        thin_edge[5] = GoldenRhombus(wide_loop[7], wide_loop[8], RhombusType::WIDE,
                        make_pair(Corner::LEFT, Corner::BOTTOM),
                        make_pair(Corner::TOP, Corner::RIGHT),
                        make_pair(Corner::TOP, Corner::LEFT), offset);
        thin_edge[6] = GoldenRhombus(wide_loop[9], wide_loop[10], RhombusType::WIDE,
                        make_pair(Corner::LEFT, Corner::BOTTOM),
                        make_pair(Corner::TOP, Corner::RIGHT),
                        make_pair(Corner::TOP, Corner::LEFT), offset);
        thin_edge[7] = GoldenRhombus(wide_loop[10], wide_loop[11], RhombusType::WIDE,
                        make_pair(Corner::LEFT, Corner::BOTTOM),
                        make_pair(Corner::TOP, Corner::RIGHT),
                        make_pair(Corner::TOP, Corner::LEFT), offset);
        thin_edge[8] = GoldenRhombus(wide_loop[12], wide_loop[13], RhombusType::WIDE,
                        make_pair(Corner::LEFT, Corner::BOTTOM),
                        make_pair(Corner::TOP, Corner::RIGHT),
                        make_pair(Corner::TOP, Corner::LEFT), offset);
        thin_edge[9] = GoldenRhombus(wide_loop[13], wide_loop[14], RhombusType::WIDE,
                        make_pair(Corner::LEFT, Corner::BOTTOM),
                        make_pair(Corner::TOP, Corner::RIGHT),
                        make_pair(Corner::TOP, Corner::LEFT), offset);
        
        all_rhombuses.reserve(35);
        for (GoldenRhombus r : center)      all_rhombuses.push_back(r);
        for (GoldenRhombus r : edges)       all_rhombuses.push_back(r);
        for (GoldenRhombus r : wide_loop)   all_rhombuses.push_back(r);
        for (GoldenRhombus r : thin_edge){
            r.skip = SkipType::SECOND;
            all_rhombuses.push_back(r);
        }
        corners[0] = wide_loop[1];
        corners[1] = wide_loop[4];
        corners[2] = wide_loop[7];
        corners[3] = wide_loop[10];
        corners[4] = wide_loop[13];
        
        assignCorners(corners, Corner::TOP);
        return;        
    }
}

RhombusIndeces GoldenRhombus::getIndeces(){
    RhombusIndeces result;
    switch (split) {
        case SplitType::LONG:
            result.triangle_a[0] = (GLuint) indeces[Corner::RIGHT];
            result.triangle_a[1] = (GLuint) indeces[Corner::TOP];
            result.triangle_a[2] = (GLuint) indeces[Corner::BOTTOM];

            result.triangle_b[0] = (GLuint) indeces[Corner::LEFT];
            result.triangle_b[1] = (GLuint) indeces[Corner::BOTTOM];
            result.triangle_b[2] = (GLuint) indeces[Corner::TOP];
            break;
        case SplitType::SHORT:
            result.triangle_a[0] = (GLuint) indeces[Corner::BOTTOM];
            result.triangle_a[1] = (GLuint) indeces[Corner::LEFT];
            result.triangle_a[2] = (GLuint) indeces[Corner::RIGHT];

            result.triangle_b[0] = (GLuint) indeces[Corner::TOP];
            result.triangle_b[1] = (GLuint) indeces[Corner::RIGHT];
            result.triangle_b[2] = (GLuint) indeces[Corner::LEFT];
            break;
    }
    return result;
}

void GoldenRhombus::writeFloats(GLfloat* start, int& head, PentagonMemory& pentagon, float texture, bool flipped){
    vec4 transformed;
    for (int i = 0; i < 4; i++) {
        if (uniques[i]) {
            transformed = vec4(corners[i].x, corners[i].y, 0.0f, 0.0f);
            transformed = pentagon.rotation*transformed;
            transformed += pentagon.offset;
            
            if(flipped) transformed -= (corners[i].z)*pentagon.normal;
            else        transformed += (corners[i].z)*pentagon.normal;
            
            start[head++] = transformed.x;
            start[head++] = transformed.y;
            start[head++] = transformed.z;
            start[head++] = transformed.w;
            
            //Texture information
            start[head++] = (corners[i].x/(DODECAPLEX_SIDE_LEN*CIRCUMRADIUS_RATIO) + 1.0f)/2.0f;
            start[head++] = (corners[i].y/(DODECAPLEX_SIDE_LEN*CIRCUMRADIUS_RATIO) + 1.0f)/2.0f;
            start[head++] = texture;
        }
    }    
}

void GoldenRhombus::writeUints(GLuint* start, int& head, uint i_offset) {
    int t_idx = 0;
    RhombusIndeces rhombus_indeces = getIndeces();
    switch (skip) {
        case SkipType::FIRST:
            start[head++] = i_offset + rhombus_indeces.triangle_b[t_idx++];
            start[head++] = i_offset + rhombus_indeces.triangle_b[t_idx++];
            start[head++] = i_offset + rhombus_indeces.triangle_b[t_idx++];
            break;
        case SkipType::SECOND:
            start[head++] = i_offset + rhombus_indeces.triangle_a[t_idx++];
            start[head++] = i_offset + rhombus_indeces.triangle_a[t_idx++];
            start[head++] = i_offset + rhombus_indeces.triangle_a[t_idx++];
            break;
        default:
            start[head++] = i_offset + rhombus_indeces.triangle_a[t_idx++];
            start[head++] = i_offset + rhombus_indeces.triangle_a[t_idx++];
            start[head++] = i_offset + rhombus_indeces.triangle_a[t_idx++];
                t_idx = 0;
            start[head++] = i_offset + rhombus_indeces.triangle_b[t_idx++];
            start[head++] = i_offset + rhombus_indeces.triangle_b[t_idx++];
            start[head++] = i_offset + rhombus_indeces.triangle_b[t_idx++];
    }
}


void RhombusWeb::buildArrays(CPUBufferPair& buffer_writer, PentagonMemory& pentagon){
    pentagon.solveRotation(web_pentagon, false);

    for (GoldenRhombus rhombus : all_rhombuses) {
        rhombus.writeFloats(buffer_writer.v_buff, buffer_writer.v_head, pentagon, web_texture, flipped);
        rhombus.writeUints( buffer_writer.i_buff, buffer_writer.i_head, buffer_writer.offset);
    }
    buffer_writer.offset += offset;
}
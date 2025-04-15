#include "rhombus.h"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <vector>

using namespace glm;

using std::pair;
using std::make_pair;
using std::array;
using std::vector;
using std::cout;
using std::endl;

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

    shared[corner.second].push_back(make_pair(&source, corner.first));
    source.shared[corner.first].push_back(make_pair(this, corner.second));
}
void GoldenRhombus::shareSide(GoldenRhombus& neighbor,
                                pair<Corner, Corner> corner_1,
                                pair<Corner, Corner> corner_2) {
    shareCorner(neighbor, corner_1);
    shareCorner(neighbor, corner_2);
    switch (corner_1.first)
    {
    case Corner::TOP:
        switch (corner_2.first)
        {
        case Corner::RIGHT:     neighbor.branches[0] = this;    break;
        case Corner::LEFT:      neighbor.branches[3] = this;    break;
        default:    throw std::invalid_argument("Bad corners");
        }
        break;
    case Corner::BOTTOM:
        switch (corner_2.first)
        {
        case Corner::RIGHT:     neighbor.branches[1] = this;    break;
        case Corner::LEFT:      neighbor.branches[2] = this;    break;
        default:    throw std::invalid_argument("Bad corners");
        }
        break;    
    case Corner::LEFT:
        switch (corner_2.first)
        {
        case Corner::TOP:       neighbor.branches[3] = this;    break;
        case Corner::BOTTOM:    neighbor.branches[2] = this;    break;
        default:    throw std::invalid_argument("Bad corners");
        }
        break;
    case Corner::RIGHT:
        switch (corner_2.first)
        {
        case Corner::TOP:       neighbor.branches[0] = this;    break;
        case Corner::BOTTOM:    neighbor.branches[1] = this;    break;
        default:    throw std::invalid_argument("Bad corners");
        }
        break;    
    default:    throw std::invalid_argument("Bad corners");
    }
    switch (corner_1.second)
    {
    case Corner::TOP:
        switch (corner_2.second)
        {
        case Corner::RIGHT:     branches[0] = &neighbor;    break;
        case Corner::LEFT:      branches[3] = &neighbor;    break;
        default:    throw std::invalid_argument("Bad corners");
        }
        break;
    case Corner::BOTTOM:
        switch (corner_2.second)
        {
        case Corner::RIGHT:     branches[1] = &neighbor;    break;
        case Corner::LEFT:      branches[2] = &neighbor;    break;
        default:    throw std::invalid_argument("Bad corners");
        }
        break;    
    default:    throw std::invalid_argument("Bad corners");
    }
};
GoldenRhombus::GoldenRhombus(GoldenRhombus& neighbor, RhombusType type, 
                                pair<Corner, Corner> corner_1,
                                pair<Corner, Corner> corner_2, uint& offset){
    vec2 rot;
    vec3 change;
    shareSide(neighbor, corner_1, corner_2);
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
    
    shareSide(neighbor_a, corner_a1, corner_a2);
    
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

void RhombusPattern::assignCorners(array<GoldenRhombus, 5>& rhombuses, Corner corner) {
    // If this is used correctly, these two values should be identical for every corner...
    pentagon_scale  = length(vec2(rhombuses[0].corners[corner]));
    vertical_offset = rhombuses[0].corners[corner].z;
    
    for (int i=0; i < 5; i++) web_pentagon[i] = vec4(vec2(rhombuses[i].corners[corner]), 0.0f, 0.0f);
    rescaleValues();
}
void RhombusPattern::assignCorners(array<GoldenRhombus*, 5> rhombuses, Corner corner) {
    // If this is used correctly, these two values should be identical for every corner...
    pentagon_scale  = length(vec2(rhombuses[0]->corners[corner]));
    vertical_offset = rhombuses[0]->corners[corner].z;
    
    for (int i=0; i < 5; i++) web_pentagon[i] = vec4(vec2(rhombuses[i]->corners[corner]), 0.0f, 0.0f);
    rescaleValues();
}
template<long unsigned int N>
void RhombusPattern::assignEdge(array<GoldenRhombus*, N> rhombuses, int edge_index){
    auto updateMap = [&](GoldenRhombus* r, Corner c){
        if (edge_map.find(r->indeces[c]) != edge_map.end()) {
            edge_map[r->indeces[c]] = make_pair(edge_index, 
                                                edge_map[r->indeces[c]].first);
        } else {
            edge_map[r->indeces[c]] = make_pair(edge_index, -1);
        }
    };
    for (GoldenRhombus* r : rhombuses) {
        edges[edge_index].push_back(r);
        if (r->split == SplitType::HORZ) {
            updateMap(r, Corner::RIGHT);
            updateMap(r, Corner::LEFT);
        } else {
            updateMap(r, Corner::TOP);
            updateMap(r, Corner::BOTTOM);
        }
    }
}
void RhombusPattern::rescaleValues(){
    for (GoldenRhombus& rhombus : all_rhombuses) {
        for (vec3& corner : rhombus.corners) {
            corner.z -= vertical_offset;
            corner *= ( (DODECAPLEX_SIDE_LEN * CIRCUMRADIUS_RATIO) / pentagon_scale );
        }
    }
}
void RhombusPattern::pushAndCount(GoldenRhombus rhombus){
    all_rhombuses.push_back(rhombus);
    if (rhombus.skip == SkipType::NONE) {
           index_count += 6;
    } else if (rhombus.skip == SkipType::BOTH) {
           index_count += 0;
    } else index_count += 3;
    for (bool unique : rhombus.uniques) {
        if (unique) vertex_count++;
    }
}
template<long unsigned int N>
void RhombusPattern::addRhombuses(std::array<GoldenRhombus, N>& rhombuses){
    for (GoldenRhombus r : rhombuses) pushAndCount(r);
}
template<long unsigned int N>
void RhombusPattern::addRhombuses(std::array<GoldenRhombus, N>& rhombuses, SkipType skip){
    for (GoldenRhombus r : rhombuses) {
        r.skip = skip;
        pushAndCount(r);
    }
}
template<long unsigned int N>
void RhombusPattern::addRhombuses(std::array<GoldenRhombus, N>& rhombuses, SkipType skip, SplitType split){
    for (GoldenRhombus r : rhombuses) {
        r.skip = skip;
        r.split = split;
        pushAndCount(r);
    }
}
template<long unsigned int N>
void RhombusPattern::addRhombuses(std::array<GoldenRhombus, N>& rhombuses, SplitType split){
    for (GoldenRhombus r : rhombuses) {
        r.split = split;
        pushAndCount(r);
    }
}
RhombusPattern::RhombusPattern(WebType pattern, bool flip) : flipped(flip) {
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
        
        addRhombuses(center);
        addRhombuses(edges, SkipType::FIRST);
        
        assignEdge( (array<GoldenRhombus*,1>) {&edges[0]}, 0);
        assignEdge( (array<GoldenRhombus*,1>) {&edges[1]}, 1);
        assignEdge( (array<GoldenRhombus*,1>) {&edges[2]}, 2);
        assignEdge( (array<GoldenRhombus*,1>) {&edges[3]}, 3);
        assignEdge( (array<GoldenRhombus*,1>) {&edges[4]}, 4);
        assignCorners(center, Corner::BOTTOM);
        break;
    case WebType::DOUBLE_STAR:
        array<GoldenRhombus, 15> wide_loop;
        array<GoldenRhombus, 10> thin_edge;

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

        all_rhombuses.reserve(25);
        
        addRhombuses(center);
        addRhombuses(edges);
        addRhombuses(wide_loop);
        addRhombuses(thin_edge, SkipType::SECOND);

        assignEdge(     (array<GoldenRhombus*, 2>) {&thin_edge[1],  &thin_edge[2]}, 0);
        assignEdge(     (array<GoldenRhombus*, 2>) {&thin_edge[3],  &thin_edge[4]}, 1);
        assignEdge(     (array<GoldenRhombus*, 2>) {&thin_edge[5],  &thin_edge[6]}, 2);
        assignEdge(     (array<GoldenRhombus*, 2>) {&thin_edge[7],  &thin_edge[8]}, 3);
        assignEdge(     (array<GoldenRhombus*, 2>) {&thin_edge[9],  &thin_edge[0]}, 4);
        assignCorners(  (array<GoldenRhombus*, 5>) {&wide_loop[1],  &wide_loop[4], &wide_loop[7], 
                                                    &wide_loop[10], &wide_loop[13]}, Corner::TOP);
        upsidedown = true;
        break;        
    }
    countVerts();
}
void RhombusPattern::countVerts(){
    for (GoldenRhombus rhombus : all_rhombuses) {
        for (bool status : rhombus.uniques) {
            if (status) num_verts++;
        }
    }
}

RhombusIndeces GoldenRhombus::getIndeces(){
    RhombusIndeces result;
    switch (split) {
        case SplitType::VERT:
            result.triangle_a[0] = (GLuint) indeces[Corner::RIGHT];
            result.triangle_a[1] = (GLuint) indeces[Corner::TOP];
            result.triangle_a[2] = (GLuint) indeces[Corner::BOTTOM];

            result.triangle_b[0] = (GLuint) indeces[Corner::LEFT];
            result.triangle_b[1] = (GLuint) indeces[Corner::BOTTOM];
            result.triangle_b[2] = (GLuint) indeces[Corner::TOP];
            break;
        case SplitType::HORZ:
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
vec4 GoldenRhombus::getTransformedCorner(enum Corner corner, PentagonMemory& pentagon, bool flip_norms){
    vec4 transformed;
    transformed = vec4(corners[corner].x, corners[corner].y, 0.0f, 0.0f);
    transformed = pentagon.rotation*transformed;
    transformed += pentagon.offset;
    
    if(flip_norms) transformed -= (corners[corner].z)*pentagon.normal;
    else           transformed += (corners[corner].z)*pentagon.normal;
    return transformed;
}
void GoldenRhombus::writeFloats(GLfloat* start, int& head, PentagonMemory& pentagon, float texture, 
                                    bool flip_norms, bool flip_text, bool write_norms){
    vec4 transformed;
    float t1, t2;
    for (int i = 0; i < 4; i++) {
        if (uniques[i]) {
            transformed = getTransformedCorner((Corner) i, pentagon, flip_norms);
            start[head++] = transformed.x;
            start[head++] = transformed.y;
            start[head++] = transformed.z;
            start[head++] = transformed.w;
            
            //Texture information
            t1 = (flip_text ? corners[i].x : -corners[i].x);
            t2 = (flip_text ? corners[i].y : -corners[i].y);           
            start[head++] = (t1/(DODECAPLEX_SIDE_LEN*CIRCUMRADIUS_RATIO) + 1.0f)/2.0f;
            start[head++] = (t2/(DODECAPLEX_SIDE_LEN*CIRCUMRADIUS_RATIO) + 1.0f)/2.0f;
            start[head++] = texture;
            
            if (write_norms) {
                start[head++] = pentagon.normal.x;
                start[head++] = pentagon.normal.y;
                start[head++] = pentagon.normal.z;
                start[head++] = pentagon.normal.w;
            }
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
        case SkipType::BOTH:
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

void GoldenRhombus::printFloats(){
    vec3 corner;
    for (int i = 0; i < 4; i++) {
        if (!uniques[i]) continue;
        corner = corners[(Corner) i];
        cout << "v";
        cout << " " << corner.x;
        cout << " " << corner.y;
        cout << " " << corner.z << endl;
    }
}
void GoldenRhombus::printUints() {
    int t_idx = 0;
    RhombusIndeces rhombus_indeces = getIndeces();
    cout << "f";
    cout << " " << rhombus_indeces.triangle_a[t_idx++]+1;
    cout << " " << rhombus_indeces.triangle_a[t_idx++]+1;
    cout << " " << rhombus_indeces.triangle_a[t_idx++]+1 << endl;;
        t_idx = 0;
    cout << "f";
    cout << " " << rhombus_indeces.triangle_b[t_idx++]+1;
    cout << " " << rhombus_indeces.triangle_b[t_idx++]+1;
    cout << " " << rhombus_indeces.triangle_b[t_idx++]+1 << endl;
}


void RhombusPattern::writeObj(){
    for (GoldenRhombus rhombus: all_rhombuses) {
        rhombus.printFloats();
        rhombus.printUints();
    }
}
void RhombusPattern::buildArrays(CPUBufferPair& buffer_writer, PentagonMemory& pentagon, bool include_normals) {
    pentagon.solveRotation(web_pentagon, false);

    for (GoldenRhombus rhombus : all_rhombuses) {
        rhombus.writeFloats(buffer_writer.v_buff, buffer_writer.v_head, pentagon, web_texture, 
                            flipped, upsidedown, include_normals);
        rhombus.writeUints( buffer_writer.i_buff, buffer_writer.i_head, buffer_writer.offset);
    }
    buffer_writer.offset += offset;
}
void RhombusPattern::buildArrays(CPUBufferPair& buffer_writer, PentagonMemory& pentagon) {
    buildArrays(buffer_writer, pentagon, false);
}
void RhombusPattern::rankVerts(mat4& player_view, PentagonMemory& pentagon) {
    vec4 result;
    int i=0;

    ranked_verts.clear();
    ranked_verts.reserve(num_verts);
    for (GoldenRhombus& rhombus : all_rhombuses){
        for (int j=0; j < 4; ++j){
            if (rhombus.uniques[j]) {
                result = player_view*rhombus.getTransformedCorner((Corner) j, pentagon, flipped);
                ranked_verts.push_back(
                    VertexRankResult(i++, length(vec2(result.x, result.y)), &rhombus, (Corner) j)
                    );
            }
        }
    }
    sort(ranked_verts.begin(), ranked_verts.end(), 
        [](VertexRankResult a, VertexRankResult b){
            return a.radius < b.radius;
        }
    );
}
void RhombusPattern::applyDamage(CPUBufferPair& buffer_writer, mat4 player_view, PentagonMemory& pentagon) {
    buffer_writer.setHead(pentagon.v_start, pentagon.i_start, pentagon.i_offset);
    rankVerts(player_view, pentagon);
    vec4 tmp;
    int off;
    for (VertexRankResult vert_data : ranked_verts){
        off = pentagon.v_start+vert_data.web_index*7;
        if (vert_data.radius > 0.3f) break;
        if (edge_map.find(vert_data.web_index) != edge_map.end()){
            if (!pentagon.neighbors[edge_map[vert_data.web_index].first].second ||
                !pentagon.neighbors[edge_map[vert_data.web_index].second].second){
                buffer_writer.v_buff[off+6] = 0.0f;
                continue;
            }
        } 
        tmp = vert_data.source->getTransformedCorner(vert_data.corner, pentagon, flipped); 
        tmp = mix(tmp, pentagon.centroids[1]*3.0f, 0.2f);        
        buffer_writer.v_buff[off++] = tmp.x;
        buffer_writer.v_buff[off++] = tmp.y;
        buffer_writer.v_buff[off++] = tmp.z;
        buffer_writer.v_buff[off++] = tmp.w;
        off++;
        off++;
        buffer_writer.v_buff[off++] = 0.0f;
    }
}

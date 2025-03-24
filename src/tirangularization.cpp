#include "triangularization.h"
#include "glm/gtx/string_cast.hpp"
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <eigen3/Eigen/Dense>

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
#define COSPF cos(PI/5.0f)
#define SINPF sin(PI/5.0f)

float Rx_WIDE(vec2 xy) {
    return COSTPF*xy.x - SINTPF*xy.y;
};
float Ry_WIDE(vec2 xy) {
    return SINTPF*xy.x + COSTPF*xy.y;
};
float Rx_THIN(vec2 xy) {
    return COSPF*xy.x - SINPF*xy.y;
};
float Ry_THIN(vec2 xy) {
    return SINPF*xy.x + COSPF*xy.y;
};

GoldenRhombus::GoldenRhombus(){};

vec3 GoldenRhombus::readCorner(Corner corner){
    return corners[corner];
}

GoldenRhombus::GoldenRhombus(RhombusType type, Corner origin, uint& offset){
    corners[origin] = vec3(0.);
    switch (type) {
        case RhombusType::WIDE:
            switch (origin) {
                case Corner::BOTTOM:
                    corners[Corner::LEFT]  = vec3(-1.0f, AY, AZ)*0.5f;
                    corners[Corner::TOP]   = vec3( 0.0f, AY, AZ);
                    corners[Corner::RIGHT] = vec3( 1.0f, AY, AZ)*0.5f;
                    break;
                case Corner::LEFT:
                    throw std::invalid_argument("Origin WIDE rhombus must use TOP/BOTTOM");
                case Corner::TOP:
                    corners[Corner::BOTTOM] = vec3( 0.0f, AY, -AZ);
                    corners[Corner::LEFT]   = vec3(-1.0f, AY, -AZ)*0.5f;
                    corners[Corner::RIGHT]  = vec3( 1.0f, AY, -AZ)*0.5f;
                    break;
                case Corner::RIGHT:
                    throw std::invalid_argument("Origin WIDE rhombus must use TOP/BOTTOM");
            }
            indeces[offset++] = Corner::BOTTOM;
            indeces[offset++] = Corner::LEFT;
            indeces[offset++] = Corner::TOP; 
            indeces[offset++] = Corner::RIGHT;
            break;
        case RhombusType::THIN:
            throw std::invalid_argument("THIN origin Rhombus origins not supported yet");
            /* switch (origin) {
                case Corner::BOTTOM:
                    corners[Corner::LEFT]  = vec3( -PHI, BX, BY)*0.5f;
                    corners[Corner::RIGHT] = vec3(  PHI, BX, BY)*0.5;
                    corners[Corner::TOP]   = vec3( 0.0f, BX, BY);
                    break;
                case Corner::TOP:
                    corners[Corner::LEFT]   = vec3( -PHI, BX, -BY)*0.5f;
                    corners[Corner::RIGHT]  = vec3(  PHI, BX, -BY)*0.5;
                    corners[Corner::BOTTOM] = vec3( 0.0f, BX, -BY);
                    break;
                default:
                    break;
            } */
    }
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
    case Corner::BOTTOM: throw std::invalid_argument("Not implemented");
        /* switch (corner_2.second) 
        {
        case Corner::LEFT: break;
        case Corner::RIGHT: break;
        default: throw std::invalid_argument("Specify horizontal destination corner with corner_2 variable");
        }
        break; */
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
            case Corner::LEFT: // TOP:RIGHT:LEFT
                corners[Corner::BOTTOM] = corners[Corner::LEFT] - change;
                indeces[Corner::BOTTOM] = offset++;
                break;
            default:
                throw std::invalid_argument("Not implemented");
            }
            break;
        default: throw std::invalid_argument("Specify horizontal destination corner with corner_2 variable");
        }
        break;
    case Corner::BOTTOM: throw std::invalid_argument("Not implemented");
    default: throw std::invalid_argument("Specify vertical destination corner with corner_1 variable");
    }

}

RhombusWeb::RhombusWeb() {
    offset = 0;
    GoldenRhombus center[5];
    // GoldenRhombus edges[5];
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
   
    /* edges[0] = GoldenRhombus(center[0], center[1], 
                    make_pair(Corner::LEFT, Corner::TOP), 
                    make_pair(Corner::BOTTOM, Corner::RIGHT), 
                    make_pair(Corner::BOTTOM, Corner::LEFT), offset);
    edges[1] = GoldenRhombus(center[1], center[2], 
                    make_pair(Corner::LEFT, Corner::TOP), 
                    make_pair(Corner::BOTTOM, Corner::RIGHT), 
                    make_pair(Corner::BOTTOM, Corner::LEFT), offset);
    edges[2] = GoldenRhombus(center[2], center[3], 
                    make_pair(Corner::LEFT, Corner::TOP), 
                    make_pair(Corner::BOTTOM, Corner::RIGHT), 
                    make_pair(Corner::BOTTOM, Corner::LEFT), offset);
    edges[3] = GoldenRhombus(center[3], center[4], 
                    make_pair(Corner::LEFT, Corner::TOP), 
                    make_pair(Corner::BOTTOM, Corner::RIGHT), 
                    make_pair(Corner::BOTTOM, Corner::LEFT), offset);
    edges[4] = GoldenRhombus(center[4], center[0], 
                    make_pair(Corner::LEFT, Corner::TOP), 
                    make_pair(Corner::BOTTOM, Corner::RIGHT), 
                    make_pair(Corner::BOTTOM, Corner::LEFT), offset); */

    collection.reserve(5);
    
    for (int i = 0; i < 5; i ++) {
        bounds[i] = center[i].readCorner(Corner::BOTTOM);
        /* scale = std::max(scale, std::abs(bounds[i].y)); */
        collection.push_back(center[i]);
    }
    scale = AY; //NOTE: not normally true...
    
    centroid_offset = length(bounds[0]-bounds[1])*(1-PHI)*sqrt(2.0f);
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

void GoldenRhombus::writeFloats(GLfloat* start, int& head, float in_scale, float out_scale,
                        mat4& rotation_mat, vec4& in_offset, vec4& out_offset){
    vec4 temp;
    for (int i = 0; i < 4; i++) {
        if (uniques[i]) {
            // reduce to xy, add in z by scaling the centroid differences????
            
            temp = vec4(corners[i], 0.0f);
            temp -= in_offset;            
            temp /= in_scale;
            temp = rotation_mat*temp;
            temp *= out_scale;

            temp += out_offset;
            
            start[head++] = temp.x;
            start[head++] = temp.y;
            start[head++] = temp.z;
            start[head++] = temp.w;
            
            start[head++] = (- corners[i].x/in_scale + 1.0f)/2.0f;
            start[head++] = (- corners[i].y/in_scale + 1.0f)/2.0f;
            start[head++] = 1.0f;


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

template <int N>
mat4 solveConversion(array<vec4,N> src, array<vec4,N> dst){
    mat4 A(0.0f);
    
    Eigen::Matrix4f eigen_mat;

    for (int i = 0; i < N; ++i) {
        vec4 s = src[i];
        vec4 d = dst[i];

        A += mat4(
            s.x * d, s.y * d, s.z * d, s.w * d
        );
    }

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            eigen_mat(i, j) = A[i][j];
        }
    }

    Eigen::JacobiSVD<Eigen::Matrix4f> svd(eigen_mat, Eigen::ComputeFullU | Eigen::ComputeFullV);

    Eigen::Matrix4f U = svd.matrixU();
    Eigen::Matrix4f Sigma = svd.singularValues().asDiagonal();
    Eigen::Matrix4f V = svd.matrixV();

    eigen_mat << 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, U.determinant()*V.determinant();
    eigen_mat = U*eigen_mat*(V.transpose());

    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            A[i][j] = eigen_mat(i, j);
        }
    }

    return A;
};

void RhombusWeb::buildArrays(GLfloat* v_buffer, GLuint* i_buffer, int& v_head, int& i_head, uint i_offset,
                                array<vec4, 5> dest_pentagon, array<vec4, 2> dest_centroids){
    array<vec4, 5> web_pentagon;

    array<vec4, 5> web_points;
    array<vec4, 5> dst_points;

    vec4 web_offset = vec4(0.0f, 0.0f, -AZ, 0.0f);
    vec4 dest_offset = vec4(0.0f);
    mat4 rotation_mat;
    float dest_scale;

    for (int i = 0; i < 5; i++){
        web_pentagon[i] = vec4(bounds[i].x, bounds[i].y, 0.0f, 0.0f);
        dest_offset += dest_pentagon[i];
    }

    dest_offset/=5.0f;

    for (int i = 0; i < 5; i++){
        dest_pentagon[i] -= dest_offset;
        dst_points[i] = dest_pentagon[i];
        web_points[i] = normalize(web_pentagon[i])*length(dst_points[i]);
    }

    dest_scale = length(dest_pentagon[0]);
    
    rotation_mat = solveConversion<5>(web_points, dst_points);
    
    std::cout << "Transofrmation error: " << std::endl;
    for (int i = 0; i < 5; i++) {
        std::cout << glm::to_string(rotation_mat*web_points[i]-dst_points[i]) << std::endl;
    }
    for (GoldenRhombus rhombus : collection) {
        rhombus.writeFloats(v_buffer, v_head, scale, dest_scale, rotation_mat, web_offset, dest_offset);
        rhombus.writeUints(i_buffer, i_head, i_offset);
    }
}

void RhombusWeb::fillCorners(array<vec4, 5>& dest) {
    for (int i = 0; i < 5; i++){
        dest[i] = vec4(bounds[i]/scale, 0.0f);
    }
}
#include "pentagon.h"
#include <Eigen/Dense>

using namespace glm;
using std::array;

array<vec4, 5> readDestinationCoords(GLuint* pentagon_ptr){
    array<vec4, 5> output;
    for (int i = 0; i < 5; i++) {
        output[i] = vec4(
            dodecaplex_cell_verts[pentagon_ptr[i]*4],
            dodecaplex_cell_verts[pentagon_ptr[i]*4+1],
            dodecaplex_cell_verts[pentagon_ptr[i]*4+2],
            dodecaplex_cell_verts[pentagon_ptr[i]*4+3]
        );
    }
    return output;
};

PentagonMemory::PentagonMemory(int side_idx) : source(side_idx) {
    GLuint* pentagon_ptr = &dodecaplex_penta_indxs[side_idx*5];

    corners   = readDestinationCoords(pentagon_ptr);
    centroids = {   dodecaplex_centroids[(side_idx/12)], 
                    dodecaplex_centroids[neighbor_side_orders[side_idx]] };
    normal    = centroids[0] - centroids[1];
    offset    = vec4(0.0f); for (vec4 corner : corners) offset+=corner;
    offset   /= 5.0f;
    centered  = {
        corners[0]-offset,
        corners[1]-offset,
        corners[2]-offset,
        corners[3]-offset,
        corners[4]-offset
    };
    
};

template <int N>
mat4 solveWithEigen(array<vec4,N> src, array<vec4,N> dst){
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

mat4 PentagonMemory::solveRotation(array<vec4, 5> start, bool force) {
    if (!has_rotation || force) {
        rotation = solveWithEigen<5>(start, centered);
        has_rotation = true;
    }
    return rotation;
};

void PentagonMemory::markStart(CPUBufferPair& bw){
    v_start =  bw.v_head;
    i_start =  bw.i_head;
    i_offset = bw.offset;
};    
void PentagonMemory::markEnd(CPUBufferPair& bw){
    v_end = bw.v_head;
    i_end = bw.i_head;
    v_len = v_end-v_start;
    i_len = i_end-i_start;
};
void PentagonMemory::addNeighbor(PentagonMemory* other, bool in_out){
    const bool pattern0[5]  = {true, true, false, false, false};
    const bool pattern1[5]  = {false, true, true, false, false};
    const bool pattern2[5]  = {false, false, true, true, false};
    const bool pattern3[5]  = {false, false, false, true, true};
    const bool pattern4[5]  = {true, false, false, false, true};
    bool matches[5]         = {false, false, false, false, false};
    bool matches_pattern[5] = {true, true, true, true, true};
    GLuint* this_ptr    = &dodecaplex_penta_indxs[source*5];
    GLuint* other_ptr   = &dodecaplex_penta_indxs[(other->source)*5];
    for (int s=0; s<5; ++s){
        for (int o=0; o<5; ++o){
            if (this_ptr[s]==other_ptr[o]) matches[s] = true;
        }
    }
    for (int i=0; i<5; ++i){
        if (matches[i] != pattern0[i]) matches_pattern[0] = false;
        if (matches[i] != pattern1[i]) matches_pattern[1] = false;
        if (matches[i] != pattern2[i]) matches_pattern[2] = false;
        if (matches[i] != pattern3[i]) matches_pattern[3] = false;
        if (matches[i] != pattern4[i]) matches_pattern[4] = false;
    }
    for (int i=0; i<5; ++i){
        if (matches_pattern[i]){
            neighbors[i] = std::make_pair(other, in_out);
            return;
        }
    }
    throw std::invalid_argument("Invalid neighbor");
}
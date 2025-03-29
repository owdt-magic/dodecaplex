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

mat4 PentagonMemory::solveRotation(array<vec4, 5> start) {
    return solveWithEigen<5>(start, centered);
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
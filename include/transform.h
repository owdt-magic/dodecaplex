#ifndef TRANSFORM_H
#define TRANSFORM_H

#define VAR_A 0.04774575
#define VAR_B 0.95225425
#define VAR_D 1.23606798
#define VAR_E 0.0954915
#define VAR_F 0.9045085
#define VAR_G 0.8090169
#define VAR_Z 0.5527864

const float conversion_matrix[9*16] = {
    -VAR_A, -VAR_A,  0.0f,  0.0f, -VAR_A,  VAR_A, 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     VAR_A, -VAR_A,  0.0f,  0.0f, -VAR_A, -VAR_A, 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     VAR_A, -VAR_A,  0.0f,  0.0f, -VAR_A,  VAR_A, 0.0f,  0.0f,  0.0f,  0.0f, -VAR_E, 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     VAR_B, -VAR_A,  0.0f,  0.0f, -VAR_A,  VAR_B, 0.0f,  0.0f,  0.0f,  0.0f,  VAR_F, 0.0f,  0.0f,  0.0f,  0.0f,  VAR_G,
     0.0f,   VAR_Z, -VAR_Z, 0.0f,  VAR_Z, -VAR_Z, 0.0f,  0.0f, -VAR_Z, 0.0f,  VAR_Z, 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     0.0f,   0.0f,   0.0f,  VAR_D, 0.0f,   0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -VAR_D, 0.0f,  0.0f,  0.0f,
    -VAR_Z,  VAR_Z,  0.0f,  0.0f,  VAR_Z,  0.0f, -VAR_Z, 0.0f,  0.0f, -VAR_Z, VAR_Z, 0.0f,  0.0f,  0.0f,  0.0f,  0.0f,
     0.0f,   0.0f,   0.0f,  0.0f,  0.0f,   0.0f,  0.0f,  VAR_D, 0.0f,  0.0f,  0.0f,  0.0f,  0.0f, -VAR_D, 0.0f,  0.0f,
     0.0f,   0.0f,   0.0f,  0.0f,  0.0f,   0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  0.0f,  VAR_D, 0.0f,  0.0f, -VAR_D, 0.0f
};

#endif
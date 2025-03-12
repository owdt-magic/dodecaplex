#include "dmath.h"

#define PI 3.1415f

#define COSPT cos(PI/10.0f)
#define SINPT sin(PI/10.0f)
#define COSPF cos(PI/5.0f)
#define SINPF sin(PI/5.0f)
#define COSTPF cos(2.0f*PI/5.0f)
#define SINTPF sin(2.0f*PI/5.0f)

#define VSCALE 1.0f
#define HSCALE (VSCALE * PHI * COSPT) / (PHI+0.5f)

GLfloat Rx(GLfloat x, GLfloat y) {
    return COSTPF*x - SINTPF*y;
};
GLfloat Ry(GLfloat x, GLfloat y) {
    return SINTPF*x + COSTPF*y;
};

struct Triangularizors {
    struct pattern_2 {
        GLfloat coordinates[46 * 2] = {0};
        GLfloat indeces[75 * 3] = {0};
        pattern_2() { populate(); };
    private:
        #define S1 COSPF / (COSPF + 1.0f)
        #define S2 1.0f / (1.0f + 2.0f*COSPF + SINPT)
        #define S3 (1.0f + COSPF) / (1.0f + 2.0f*COSPF + SINPT)
        #define S4 SINPF / (2.0f*SINPF + COSPT)
        #define S5 (2.0f*COSPF + 1.0f) / (2.0f*COSPF + 2.0f)
        #define S6 (2.0f*COSPF + 1.0f) / (2.0f*COSPF + 1.0f + SINPT)
        #define S7 COSPT / (COSPT + 2.0f*SINPF)
        GLfloat src_pts[9*2] = {
            0.0f,      VSCALE*S2,
            0.0f,      VSCALE*S6, 
            HSCALE*S1, VSCALE*S1,
            HSCALE*S5, VSCALE*S5,
            HSCALE,    VSCALE,
            HSCALE*S4, VSCALE*S3,
            -HSCALE*S4, VSCALE*S3,
            HSCALE*S7, VSCALE,
            -HSCALE*S7, VSCALE
        };
        void populate(){
            int c = 2; //Skip first coords, which remain zero
            int i = 0;
            int off;
            GLfloat* index_ptr = indeces;
            for (int f = 0; f < 5; f++) {
                for (int s = 0; s < 9; s++) {
                    coordinates[c++] = src_pts[s * 2];
                    coordinates[c++] = src_pts[s * 2 + 1];
                    src_pts[s * 2] = Rx(src_pts[s * 2], src_pts[s * 2 + 1]);
                    src_pts[s * 2 + 1] = Ry(src_pts[s * 2], src_pts[s * 2 + 1]);
                }

                off = f * 9;

                int temp_subindeces[15 * 3] = {
                    // ORANGE
                    0, off + 1, (off + 9) % 45 + 1,
                    off + 1, off + 6, off + 7,
                    off + 2, off + 6, off + 7,
                    off + 2, off + 8, off + 9,
                    // GREEN
                    off + 1, off + 3, off + 6,
                    off + 3, off + 4, off + 6,
                    off + 4, off + 5, off + 8,
                    // PURPLE
                    off + 4, off + 8, off + 2,
                    off + 4, off + 2, off + 6,
                    (off + 3) % 45 + 1, off + 9, off + 2,
                    (off + 3) % 45 + 1, off + 2, off + 7,
                    // PINK
                    off + 1, (off + 2) % 45 + 1, off + 7,
                    (off + 2) % 45 + 1, off + 7, (off + 3) % 45 + 1,
                    (off + 3) % 45 + 1, off + 9, (off + 4) % 45 + 1,
                    // BLUE
                    off + 1, (off + 9) % 45 + 1, (off + 2) % 45 + 1
                };

                std::memcpy(index_ptr, temp_subindeces, sizeof(temp_subindeces));

                index_ptr += (15 * 3);
            }
        }
    };
};
struct Sigil{
    GLfloat* verts;
    GLuint* indeces;
    int vert_len;
    int indx_len;
    Sigil(GLfloat* v, GLuint* i, int vl, int il) :
        verts(v), indeces(i), vert_len(vl), indx_len(il) {};
};

GLfloat mining_sigil_vs[] = {
    0.273f, 0.195f,
    0.33f, 0.422f,
    0.398f, 0.403f,
    0.252f, 0.0417f,
    0.39f, 0.332f,
    0.517f, 0.392f,
    0.357f, 0.532f,
    0.612f, 0.66f,
    0.725f, 0.768f,
    0.78f, 0.962f,
    0.577f, 0.725f,
    0.522f, 0.638f,
    0.588f, 0.538f,
    0.415f, 0.622f,
    0.507f, 0.355f,
    0.583f, 0.408f,
    0.632f, 0.362f,
    0.662f, 0.422f,
    0.615f, 0.458f,
    0.637f, 0.522f,
    0.335f, 0.52f,
    0.335f, 0.578f,
    0.26f, 0.59f,
    0.438f, 0.672f,
    0.375f, 0.637f,
    0.348f, 0.695f,
    0.752f, 0.33f,
    0.238f, 0.648f,
    0.158f, 0.708f,
    0.477f, 0.287f,
    0.573f, 0.313f,
    0.562f, 0.255f,
    0.473f, 0.243f,
    0.533f, 0.2f,
    0.618f, 0.138f,
    0.392f, 0.72f,
    0.475f, 0.722f,
    0.395f, 0.763f,
    0.485f, 0.79f,
    0.383f, 0.802f,
    0.405f, 0.848f,
    0.42f, 0.905f,
    0.688f, 0.493f,
    0.652f, 0.583f,
    0.825f, 0.537f,
    0.287f, 0.408f,
    0.297f, 0.498f,
    0.0933f, 0.435f,
};
GLuint mining_sigil_is[] = {
    0, 1, 2,
    0, 3, 4,
    2, 4, 0,
    4, 5, 2,
    2, 6, 1,
    7, 8, 9,
    7, 10, 9,
    7, 11, 10,
    11, 12, 7,
    10, 13, 11,
    14, 15, 16,
    16, 17, 15,
    15, 18, 17,
    17, 19, 18,
    20, 21, 22,
    23, 24, 25,
    24, 22, 21,
    25, 24, 22,
    16, 26, 17,
    22, 27, 25,
    25, 28, 27,
    29, 30, 31,
    29, 32, 31,
    32, 33, 31,
    31, 34, 33,
    35, 36, 37,
    37, 38, 36,
    38, 39, 37,
    39, 40, 38,
    38, 41, 40,
    42, 43, 44,
    45, 46, 47
};
GLfloat building_sigil_vs[] = {
    0.513f, 0.233f,
    0.432f, 0.198f,
    0.503f, 0.18f,
    0.37f, 0.158f,
    0.318f, 0.232f,
    0.27f, 0.215f,
    0.268f, 0.335f,
    0.31f, 0.368f,
    0.282f, 0.398f,
    0.387f, 0.417f,
    0.417f, 0.498f,
    0.478f, 0.493f,
    0.44f, 0.605f,
    0.498f, 0.655f,
    0.498f, 0.732f,
    0.687f, 0.782f,
    0.703f, 0.723f,
    0.75f, 0.738f,
    0.713f, 0.623f,
    0.765f, 0.683f,
    0.773f, 0.555f,
    0.713f, 0.525f,
    0.747f, 0.45f,
    0.765f, 0.388f,
    0.793f, 0.393f,
    0.29f, 0.447f,
    0.29f, 0.578f,
    0.227f, 0.58f,
    0.315f, 0.62f,
    0.27f, 0.655f,
    0.417f, 0.638f,
    0.188f, 0.57f,
    0.175f, 0.682f,
    0.14f, 0.677f,
    0.222f, 0.773f,
    0.222f, 0.82f,
    0.317f, 0.742f,
    0.305f, 0.137f,
    0.218f, 0.272f,
    0.193f, 0.123f,
    0.615f, 0.833f,
    0.778f, 0.803f,
    0.738f, 0.908f,
    0.488f, 0.372f,
    0.562f, 0.437f,
    0.52f, 0.337f,
    0.657f, 0.475f,
    0.617f, 0.517f,
    0.687f, 0.238f,
    0.733f, 0.28f,
    0.777f, 0.19f
};
GLuint building_sigil_is[] = {
    0, 1, 2,
    2, 1, 3,
    1, 3, 4,
    4, 3, 5,
    5, 4, 6,
    4, 6, 7,
    6, 7, 8,
    8, 7, 9,
    8, 9, 10,
    9, 11, 10,
    10, 12, 11,
    11, 12, 13,
    12, 13, 14,
    13, 14, 15,
    13, 15, 16,
    16, 15, 17,
    17, 16, 18,
    18, 17, 19,
    19, 18, 20,
    20, 18, 21,
    21, 20, 22,
    22, 21, 23,
    22, 23, 24,
    25, 26, 27,
    26, 28, 29,
    29, 26, 27,
    28, 30, 29,
    31, 32, 33,
    32, 33, 34,
    34, 33, 35,
    34, 36, 35,
    37, 38, 39,
    40, 41, 42,
    43, 44, 45,
    44, 46, 47,
    44, 48, 49,
    49, 50, 48
};
GLfloat writing_vs[] = {
    0.0883f, 0.06f,
    0.085f, 0.145f,
    0.1f, 0.0933f,
    0.102f, 0.127f,
    0.125f, 0.145f,
    0.147f, 0.143f,
    0.165f, 0.125f,
    0.153f, 0.055f,
    0.107f, 0.0433f,
    0.133f, 0.0417f,
    0.122f, 0.09f,
    0.2f, 0.055f,
    0.223f, 0.055f,
    0.208f, 0.157f,
    0.243f, 0.137f,
    0.272f, 0.153f,
    0.267f, 0.09f,
    0.292f, 0.115f,
    0.31f, 0.0633f,
    0.338f, 0.0383f,
    0.423f, 0.05f,
    0.352f, 0.07f,
    0.373f, 0.055f,
    0.38f, 0.145f,
    0.365f, 0.16f,
    0.495f, 0.0467f,
    0.475f, 0.0567f,
    0.468f, 0.118f,
    0.493f, 0.167f,
    0.473f, 0.153f,
    0.522f, 0.0467f,
    0.553f, 0.08f,
    0.515f, 0.095f,
    0.543f, 0.147f,
    0.628f, 0.123f,
    0.63f, 0.15f,
    0.613f, 0.112f,
    0.628f, 0.035f,
    0.548f, 0.04f,
    0.553f, 0.0567f,
    0.588f, 0.0483f,
    0.188f, 0.207f,
    0.187f, 0.277f,
    0.215f, 0.248f,
    0.257f, 0.19f,
    0.232f, 0.203f,
    0.267f, 0.232f,
    0.245f, 0.262f,
    0.213f, 0.28f,
    0.273f, 0.272f,
    0.275f, 0.253f,
    0.237f, 0.29f,
    0.257f, 0.29f,
    0.075f, 0.217f,
    0.0617f, 0.233f,
    0.135f, 0.233f,
    0.153f, 0.213f,
    0.337f, 0.225f,
    0.383f, 0.178f,
    0.372f, 0.208f,
    0.44f, 0.215f,
    0.407f, 0.207f,
    0.36f, 0.273f,
    0.363f, 0.245f,
    0.427f, 0.275f,
    0.398f, 0.263f,
    0.427f, 0.245f,
    0.538f, 0.192f,
    0.515f, 0.227f,
    0.552f, 0.217f,
    0.52f, 0.282f,
    0.603f, 0.278f,
    0.603f, 0.257f,
    0.572f, 0.2f,
    0.567f, 0.225f,
    0.617f, 0.212f,
    0.588f, 0.247f,
    0.563f, 0.255f,
    0.663f, 0.193f,
    0.697f, 0.19f,
    0.683f, 0.235f,
    0.65f, 0.218f,
    0.663f, 0.24f,
    0.652f, 0.278f,
    0.68f, 0.268f,
    0.708f, 0.278f,
    0.728f, 0.272f,
    0.747f, 0.25f,
    0.725f, 0.187f,
    0.823f, 0.19f,
    0.78f, 0.223f,
    0.8f, 0.222f,
    0.822f, 0.258f,
    0.845f, 0.183f,
    0.847f, 0.27f,
    0.888f, 0.223f,
    0.0817f, 0.377f,
    0.0983f, 0.355f,
    0.398f, 0.343f,
    0.335f, 0.363f,
    0.448f, 0.312f,
    0.432f, 0.35f,
    0.437f, 0.397f,
    0.455f, 0.412f,
    0.388f, 0.41f,
    0.492f, 0.317f,
    0.495f, 0.362f,
    0.513f, 0.338f,
    0.52f, 0.402f,
    0.543f, 0.342f,
    0.558f, 0.308f,
    0.565f, 0.357f,
    0.542f, 0.412f,
    0.0717f, 0.482f,
    0.0967f, 0.463f,
    0.35f, 0.482f,
    0.372f, 0.453f,
    0.412f, 0.433f,
    0.415f, 0.495f,
    0.433f, 0.443f,
    0.443f, 0.517f,
    0.465f, 0.443f,
    0.485f, 0.425f,
    0.492f, 0.493f,
    0.467f, 0.512f,
    0.513f, 0.433f,
    0.538f, 0.453f,
    0.525f, 0.525f,
    0.545f, 0.488f,
    0.618f, 0.515f,
    0.625f, 0.337f,
    0.625f, 0.383f,
    0.603f, 0.358f,
    0.608f, 0.473f,
    0.638f, 0.447f,
    0.638f, 0.49f,
    0.628f, 0.355f,
    0.738f, 0.352f,
    0.728f, 0.373f,
    0.642f, 0.467f,
    0.732f, 0.45f,
    0.738f, 0.472f,
    0.75f, 0.408f,
    0.0833f, 0.567f,
    0.0833f, 0.712f,
    0.108f, 0.587f,
    0.113f, 0.643f,
    0.102f, 0.71f,
    0.182f, 0.707f,
    0.125f, 0.625f,
    0.133f, 0.585f,
    0.173f, 0.555f,
    0.183f, 0.677f,
    0.218f, 0.557f,
    0.33f, 0.545f,
    0.223f, 0.6f,
    0.232f, 0.618f,
    0.34f, 0.568f,
    0.307f, 0.653f,
    0.342f, 0.598f,
    0.313f, 0.693f,
    0.358f, 0.688f,
    0.337f, 0.673f,
    0.242f, 0.698f,
    0.238f, 0.645f,
    0.287f, 0.662f,
    0.45f, 0.563f,
    0.59f, 0.557f,
    0.545f, 0.583f,
    0.537f, 0.602f,
    0.54f, 0.682f,
    0.46f, 0.677f,
    0.452f, 0.595f,
    0.56f, 0.6f,
    0.558f, 0.673f,
    0.585f, 0.673f,
    0.37f, 0.557f,
    0.417f, 0.555f,
    0.393f, 0.685f,
    0.428f, 0.682f,
    0.107f, 0.745f,
    0.112f, 0.837f,
    0.128f, 0.765f,
    0.137f, 0.803f,
    0.16f, 0.823f,
    0.137f, 0.737f,
    0.157f, 0.773f,
    0.165f, 0.727f,
    0.187f, 0.742f,
    0.185f, 0.82f,
    0.202f, 0.798f,
    0.233f, 0.738f,
    0.255f, 0.737f,
    0.25f, 0.817f,
    0.283f, 0.797f,
    0.317f, 0.813f,
    0.287f, 0.765f,
    0.327f, 0.767f,
    0.348f, 0.737f,
    0.458f, 0.728f,
    0.403f, 0.718f,
    0.415f, 0.742f,
    0.402f, 0.803f,
    0.395f, 0.748f,
    0.43f, 0.815f,
    0.525f, 0.72f,
    0.502f, 0.735f,
    0.502f, 0.788f,
    0.515f, 0.827f,
    0.533f, 0.827f,
    0.563f, 0.713f,
    0.578f, 0.815f,
    0.592f, 0.703f,
    0.622f, 0.703f,
    0.643f, 0.703f,
    0.643f, 0.805f,
    0.673f, 0.81f,
    0.662f, 0.707f,
    0.697f, 0.705f,
    0.712f, 0.805f,
    0.738f, 0.805f,
    0.723f, 0.708f
};
GLuint writing_is[] = {
    0, 1, 2,
    2, 1, 3,
    3, 1, 4,
    5, 6, 7,
    8, 9, 10,
    11, 12, 13,
    13, 14, 15,
    15, 16, 17,
    18, 19, 20,
    21, 22, 23,
    24, 23, 21,
    25, 26, 27,
    25, 27, 28,
    28, 29, 27,
    30, 31, 30,
    31, 30, 32,
    33, 34, 35,
    34, 36, 37,
    38, 38, 39,
    40, 38, 38,
    39, 40, 38,
    41, 42, 43,
    44, 45, 46,
    45, 46, 47,
    48, 49, 50,
    47, 51, 52,
    53, 54, 55,
    55, 56, 53,
    57, 58, 59,
    58, 60, 61,
    57, 62, 63,
    62, 64, 65,
    60, 64, 66,
    67, 68, 69,
    69, 68, 70,
    70, 71, 72,
    73, 74, 75,
    75, 72, 71,
    74, 76, 77,
    78, 79, 80,
    81, 82, 83,
    83, 82, 84,
    83, 84, 85,
    86, 87, 88,
    89, 90, 91,
    91, 92, 90,
    93, 94, 95,
    96, 97, 98,
    96, 98, 99,
    100, 101, 102,
    102, 100, 103,
    103, 102, 104,
    105, 106, 107,
    107, 106, 108,
    109, 110, 111,
    109, 112, 111,
    113, 114, 115,
    115, 114, 116,
    117, 118, 119,
    119, 118, 120,
    121, 122, 123,
    121, 124, 123,
    125, 126, 127,
    126, 128, 127,
    128, 127, 129,
    130, 131, 132,
    133, 134, 135,
    136, 137, 138,
    139, 140, 141,
    138, 140, 142,
    143, 144, 145,
    146, 147, 148,
    149, 150, 151,
    152, 149, 151,
    153, 154, 155,
    156, 157, 158,
    159, 160, 161,
    160, 162, 163,
    163, 164, 165,
    166, 167, 168,
    166, 169, 170,
    170, 171, 172,
    167, 173, 174,
    174, 175, 167,
    176, 177, 178,
    178, 179, 177,
    180, 181, 182,
    182, 183, 181,
    183, 184, 181,
    185, 186, 187,
    188, 189, 190,
    191, 192, 193,
    193, 194, 195,
    195, 196, 197,
    198, 199, 200,
    201, 202, 203,
    201, 204, 202,
    205, 206, 207,
    207, 208, 209,
    205, 207, 209,
    210, 211, 212,
    213, 214, 215,
    216, 217, 218,
    219, 220, 221
};

Sigil all_sigils[] = {
    Sigil(mining_sigil_vs, mining_sigil_is, 
        sizeof(mining_sigil_vs)/sizeof(GLfloat),
        sizeof(mining_sigil_is)/sizeof(GLuint)),
    Sigil(building_sigil_vs, building_sigil_is, 
        sizeof(building_sigil_vs)/sizeof(GLfloat),
        sizeof(building_sigil_is)/sizeof(GLuint)),
};

Sigil writing = Sigil(writing_vs, writing_is,
                    sizeof(writing_vs)/sizeof(GLfloat),
                    sizeof(writing_is)/sizeof(GLuint));
#include "config.h"
#include <glad/glad.h>
#include <stb/stb_image.h>
#include <stdexcept>
#include <array>

struct TextureLibrary {
    static const int PENT_COUNT = 4;
    static const int SPEC_COUNT = 3;
    const char* pentagon_paths[PENT_COUNT] = {
        TEXTURE_DIR "/curved.png",
        TEXTURE_DIR "/tile_floor_b.png",
        TEXTURE_DIR "/tile_floor_a.png",
        TEXTURE_DIR "/test_text.png"
    };
    const char* specular_paths[SPEC_COUNT] = {
        TEXTURE_DIR "/curved_spec.png",
        TEXTURE_DIR "/tile_floor_b_disp.png",
        TEXTURE_DIR "/tile_floor_a.png"
    };
    const char* grimoire_paths[2] = {
        TEXTURE_DIR "/grimoire_cover.png",
        TEXTURE_DIR "/grimoire_page_1.png"
    };
    void linkPentagonLibrary(GLuint shaderID);
    void linkGrimoireLibrary(GLuint shaderID);
    
    void readRGBATextureArray(const char* paths[], int num_imgs, int prog_index);
};
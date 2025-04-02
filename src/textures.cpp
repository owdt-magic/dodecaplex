#define STB_IMAGE_IMPLEMENTATION
#include "textures.h"
#include <iostream>

void TextureLibrary::readRGBATextureArray(const char* paths[], int num_imgs, int prog_index) {
    GLuint textureArrayID;
    glGenTextures(1, &textureArrayID);
    glActiveTexture(GL_TEXTURE0+prog_index);
    glBindTexture(GL_TEXTURE_2D_ARRAY, textureArrayID);
    
    int width, height, channels;
    unsigned char* img_data;
    for (int i = 0; i < num_imgs; i++) {
        img_data = stbi_load(paths[i], &width, &height, &channels, STBI_rgb_alpha);
        if (!img_data) throw std::runtime_error("Missing texture image file");
        if (!i) glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, width, height, num_imgs, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, img_data);
        stbi_image_free(img_data);
        img_data = NULL; // Ensures catching of failed loads.
    }
    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void TextureLibrary::linkPentagonLibrary(GLuint shaderID) {
    GLuint location;
    
    readRGBATextureArray(pentagon_paths, PENT_COUNT, 0);
    location = glGetUniformLocation(shaderID, "pentagonTextures");        
    glUniform1i(location, 0);

    readRGBATextureArray(specular_paths, SPEC_COUNT, 1);
    location = glGetUniformLocation(shaderID, "specularTextures");
    glUniform1i(location, 1);

    readRGBATextureArray(spell_paths, 1, 3);
    location = glGetUniformLocation(shaderID, "spellTextures");
    glUniform1i(location, 3);

}

void TextureLibrary::linkGrimoireLibrary(GLuint shaderID) {
    GLuint location;
    readRGBATextureArray(grimoire_paths, 2, 2);
    location = glGetUniformLocation(shaderID, "grimoireTextures");
    glUniform1i(location, 2);
}
#include "spells.h"
#include <iostream>

void teleportA(float progress, glm::vec3 start, glm::vec3 start_up, glm::vec3 target, int target_index, WorldCell* cell, PlayerContext* context){
    static glm::vec3 target_head;
    static glm::vec3 direction;
    target_head = target+cell->floor_norms[target_index]*context->player_location->getHeight();
    direction = normalize(target_head-start)*0.01f;
    context->player_location->teleportHead(mix(target_head, start, progress));
    context->player_location->teleportPUp(mix(cell->floor_norms[target_index], start_up, progress));
};
void teleportAStart(glm::vec3 start, glm::vec3 start_up, glm::vec3 target, int target_index, WorldCell* cell, PlayerContext* context){
    context->player_location->setFloorIndex(target_index);
    context->player_location->reference_cell = cell;
};

void SpellLog::linkGrimoireVAOs(){
    grimoire_vao.LinkAttrib(grimoire_vao.vbo, 0, 2, GL_FLOAT, 6 * sizeof(float), (void*)0); // Position attribute
    grimoire_vao.LinkAttrib(grimoire_vao.vbo, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Texture coord attribute
}
void SpellLog::drawGrimoireVAOs(){
    grimoire_vao.DrawElements(GL_TRIANGLES);
};

GLfloat SpellLog::curved_page_verts[PAGE_LOD*6*2];
GLuint SpellLog::curved_page_indeces[PAGE_LOD*2*3];

void SpellLog::populateCurvedPageData() {
    float upper_bound = 0.0f;
    float lower_bound = -1.0f;
    float left_bound  = 0.0f;
    float right_bound = 1.0f;
    float depth = 0.75f;
    float inter_vert, inter_text;

/*     // First two vertices
    curved_page_verts[0] = left_bound;
    curved_page_verts[1] = upper_bound;
    curved_page_verts[2] = depth;
    curved_page_verts[3] = 0.0f;
    curved_page_verts[4] = 0.0f;
    curved_page_verts[5] = 0.0f;

    curved_page_verts[6] = left_bound;
    curved_page_verts[7] = lower_bound;
    curved_page_verts[8] = depth;
    curved_page_verts[9] = 0.0f;
    curved_page_verts[10] = 1.0f;
    curved_page_verts[11] = 0.0f; */

    for (int i = 0; i < PAGE_LOD+1; i++) {
        inter_vert = glm::mix(left_bound, right_bound, float(i) / float(PAGE_LOD));
        inter_text = glm::mix(0.0f, 1.0f, float(i) / float(PAGE_LOD));
        
        curved_page_verts[6 * 2 * i] = inter_vert;
        curved_page_verts[6 * 2 * i + 1] = upper_bound;
        curved_page_verts[6 * 2 * i + 2] = depth;
        curved_page_verts[6 * 2 * i + 3] = inter_text;
        curved_page_verts[6 * 2 * i + 4] = 0.0f;
        curved_page_verts[6 * 2 * i + 5] = 0.0f;

        curved_page_verts[6 * 2 * i + 6] = inter_vert;
        curved_page_verts[6 * 2 * i + 7] = lower_bound;
        curved_page_verts[6 * 2 * i + 8] = depth;
        curved_page_verts[6 * 2 * i + 9] = inter_text;
        curved_page_verts[6 * 2 * i + 10] = 1.0f;
        curved_page_verts[6 * 2 * i + 11] = 0.0f;

        curved_page_indeces[6 * i - 6] = (i - 1) * 2;
        curved_page_indeces[6 * i - 5] = (i - 1) * 2 + 1;
        curved_page_indeces[6 * i - 4] = (i - 1) * 2 + 2;

        curved_page_indeces[6 * i - 3] = (i - 1) * 2 + 1;
        curved_page_indeces[6 * i - 2] = (i - 1) * 2 + 2;
        curved_page_indeces[6 * i - 1] = (i - 1) * 2 + 3;
    }

    GLfloat* vert_ptr = curved_page_verts;
    GLuint* indx_ptr = curved_page_indeces;
    GLsizei vert_size = sizeof(curved_page_verts);
    GLsizei indx_size = sizeof(curved_page_indeces);

    grimoire_vao = VAO(vert_ptr, vert_size, indx_ptr, indx_size);

}

SpellLog::SpellLog() {
    // Initialize other members...
    populateCurvedPageData();
}
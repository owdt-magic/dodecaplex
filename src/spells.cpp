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
    float upper_bound = 0.0f,
          lower_bound = -1.0f,
          left_bound  = 0.0f,
          right_bound = 1.0f,
          depth = 0.75f;
          
    float inter_vert, inter_text;
    int vi = 0, pi = 0;

    for (int i = 0; i < PAGE_LOD; i++) {
        inter_vert = glm::mix(left_bound, right_bound, float(i) / float(PAGE_LOD-1));
        inter_text = glm::mix(0.0f, 1.0f, float(i) / float(PAGE_LOD-1));
        
        curved_page_verts[vi++] = inter_vert;
        curved_page_verts[vi++] = upper_bound;
        curved_page_verts[vi++] = depth;
        curved_page_verts[vi++] = inter_text;
        curved_page_verts[vi++] = 0.0f;
        curved_page_verts[vi++] = 0.0f;

        curved_page_verts[vi++] = inter_vert;
        curved_page_verts[vi++] = lower_bound;
        curved_page_verts[vi++] = depth;
        curved_page_verts[vi++] = inter_text;
        curved_page_verts[vi++] = 1.0f;
        curved_page_verts[vi++] = 0.0f;

        if (i != 0) {            
            curved_page_indeces[pi++] = (vi - 12)/6;
            curved_page_indeces[pi++] = (vi - 18)/6;
            curved_page_indeces[pi++] = (vi - 24)/6;

            curved_page_indeces[pi++] = (vi - 6)/6;
            curved_page_indeces[pi++] = (vi - 12)/6;
            curved_page_indeces[pi++] = (vi - 18)/6;
        }
    }

    grimoire_vao = VAO(curved_page_verts, sizeof(curved_page_verts), curved_page_indeces, sizeof(curved_page_indeces));

}

SpellLog::SpellLog() {
    // Initialize other members...
    populateCurvedPageData();
}
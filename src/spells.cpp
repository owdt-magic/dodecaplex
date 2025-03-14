#include "spells.h"

using namespace glm;

void teleportA(SpellContext sc){
    static vec3 target_head;
    static vec3 direction;
    target_head = glm::cross(sc.start, sc.start_up);
    direction = normalize(target_head-sc.start)*0.01f;
    sc.context->player_location->teleportHead(mix(target_head, sc.start, sc.progress));
};
void teleportAStart(SpellContext sc){
    //sc.context->player_location->setFloorIndex(sc.target_index);
    //TODO: Temporary omission. Spells don't work in new 4D version...    
};

void Grimoire::updateSpellLife(float time, PlayerContext* player_context) {
    spell_life[active_spell] = 1.0f-std::max(0.0f, std::min(1.0f, (time-release_times[active_spell])/spell_durrations[active_spell]));
    updateSpellFunction[active_spell](
        SpellContext(            
            spell_head          [active_spell], 
            cast_player_up      [active_spell], player_context,
            spell_life          [active_spell]
        )
    );
};
void Grimoire::startSpell(float time, vec3 focus, vec3 head, vec3 player_up, PlayerContext* player_context) {
    spell_life          [active_spell] = 1.0f;
    release_times       [active_spell] = time;
    spell_focus         [active_spell] = focus;
    spell_head          [active_spell] = head;
    cast_player_up      [active_spell] = player_up;
    click_times         [active_spell] = 0.0f;
    startSpellFunction[active_spell](
        SpellContext(
            spell_head          [active_spell], 
            cast_player_up      [active_spell],
            player_context, 0.0f
        )
    );

};
void Grimoire::chargeSpell(float time, vec3 focus, vec3 head){
    cast_life   [active_spell] = std::max(0.0f, std::min(1.0f, (time-click_times[active_spell])/cast_durrations[active_spell]));
    spell_focus [active_spell] = focus;
    spell_head  [active_spell] = head;
};
void Grimoire::flipRight(float time){
    flip_direction = true;
    flip_start = time;
    flip_progress = -1.0f;
};
void Grimoire::flipLeft(float time){
    flip_direction = false;
    flip_start = time;
    flip_progress = 1.0f;    
};
void Grimoire::updateFlip(float time){
    static float start = -1.0f;
    static float end = 1.0f;
    flip_progress = mix(start, end, (time-flip_start)/flip_durration);
    if (flip_progress > end) {
        flip_progress = 0.0f;
    } else {
        if (!flip_direction) flip_progress *= -1.0f;
    }
};
void Grimoire::linkGrimoireVAOs(){
    pages_vao.LinkAttrib(pages_vao.vbo, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0); // Position attribute
    pages_vao.LinkAttrib(pages_vao.vbo, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Texture coord attribute
};
void Grimoire::drawGrimoireVAOs(GLuint flip_uniform_index){
    glDisable(GL_DEPTH_TEST);
    glUniform1f(flip_uniform_index, -1.0f);
    pages_vao.DrawElements(GL_TRIANGLES);
    glUniform1f(flip_uniform_index, 1.0f);
    pages_vao.DrawElements(GL_TRIANGLES);
    if (flip_progress != 0.0f) {
        glUniform1f(flip_uniform_index, flip_progress);
        pages_vao.DrawElements(GL_TRIANGLES);
    }
    glEnable(GL_DEPTH_TEST);
    
};

GLfloat Grimoire::curved_page_verts[PAGE_LOD*6*2];
GLuint Grimoire::curved_page_indeces[PAGE_LOD*2*3];

void Grimoire::populateCurvedPageData() {
    static float upper_bound = 0.0f,
                lower_bound = 1.0f,
                left_bound  = 0.0f,
                right_bound = 1.0f;

    static float texture_top = 0.044f,
                texture_bottom = 0.9531f,
                texture_left = 0.1875f,
                texture_right = 0.8691f;

    static float inter_vert, inter_text, depth;
    static int vi = 0, pi = 0;

    for (int i = 0; i < PAGE_LOD; i++) {
        inter_vert = mix(left_bound, right_bound, float(i) / float(PAGE_LOD-1));
        inter_text = mix(texture_left, texture_right, float(i) / float(PAGE_LOD-1));
        depth = sin((float(i) / float(PAGE_LOD-1))*3.14f)*-0.3f;
        
        curved_page_verts[vi++] = inter_vert;
        curved_page_verts[vi++] = upper_bound;
        curved_page_verts[vi++] = depth;
        curved_page_verts[vi++] = inter_text;
        curved_page_verts[vi++] = texture_top;
        curved_page_verts[vi++] = 1.0f;

        curved_page_verts[vi++] = inter_vert;
        curved_page_verts[vi++] = lower_bound;
        curved_page_verts[vi++] = depth;
        curved_page_verts[vi++] = inter_text;
        curved_page_verts[vi++] = texture_bottom;
        curved_page_verts[vi++] = 1.0f;

        if (i != 0) {            
            curved_page_indeces[pi++] = (vi - 12)/6;
            curved_page_indeces[pi++] = (vi - 18)/6;
            curved_page_indeces[pi++] = (vi - 24)/6;

            curved_page_indeces[pi++] = (vi - 6)/6;
            curved_page_indeces[pi++] = (vi - 12)/6;
            curved_page_indeces[pi++] = (vi - 18)/6;
        }
    }

    pages_vao = VAO(curved_page_verts, sizeof(curved_page_verts), curved_page_indeces, sizeof(curved_page_indeces));

}

Grimoire::Grimoire() {
    // Initialize other members...
    populateCurvedPageData();
}
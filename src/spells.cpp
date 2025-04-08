#include "spells.h"
#include "sigils.h"

using namespace glm;

void Spell::reset(){
    click_time = 0.0f;
    release_time = 0.0f;
    spell_life = 0.0f; 
    cast_life = 0.0f;
};

void miningSpell(PlayerContext* pc, Spell* spell){
    pc->elapseShrapnel(spell->spell_life);
};
void buildingSpell(PlayerContext* pc, Spell* spell){
    if (spell->spell_life == 1.0f) pc->elapseGrowth(spell->spell_life);
 };
void emptySpell(PlayerContext* pc, Spell* spell){ };

void Grimoire::updateSpellLife(float time, PlayerContext* player_context) {
    active_spell->updateSpellFunction(player_context, active_spell);
    active_spell->spell_life = 1.0f-std::max(0.0f, std::min(1.0f, (time-active_spell->release_time)/active_spell->spell_durration));
};
void Grimoire::startSpell(float time, vec3 focus, vec3 head, vec3 player_up, PlayerContext* player_context) {
    active_spell->spell_life      = 1.0f;
    active_spell->release_time    = time;
    active_spell->spell_focus     = focus;
    active_spell->spell_head      = head;
    active_spell->cast_player_up  = player_up;
    active_spell->click_time      = 0.0f;
    active_spell->startSpellFunction(player_context, active_spell);
};
void Grimoire::chargeSpell(float time, vec3 focus, vec3 head){
    active_spell->cast_life   = std::max(0.0f, std::min(1.0f, (time-active_spell->click_time)/active_spell->cast_durration));
    active_spell->spell_focus = focus;
    active_spell->spell_head  = head;
};
void Grimoire::flipRight(float time){
    if (time-flip_start > 0.5f) {
        flip_direction = true;
        flip_start = time;
        flip_progress = -1.0f;
        (page == SPELL_COUNT-1) ? page = 0 : page++;
        active_spell = &all_spells.at(page);
        active_spell->reset();
    }
};
void Grimoire::flipLeft(float time){
    if (time-flip_start > 0.5f) {
        flip_direction = false;
        flip_start = time;
        flip_progress = 1.0f;
        (page == 0) ? page = SPELL_COUNT-1: page--;
        active_spell = &all_spells.at(page);
        active_spell->reset();
    }
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
void Grimoire::drawGrimoireVAOs(GLuint flip_uniform_index){
    glDisable(GL_DEPTH_TEST);
    glUniform1f(flip_uniform_index, -1.0f);
    pages_vao.DrawElements(GL_TRIANGLES);
    glUniform1f(flip_uniform_index, 1.0f);
    pages_vao.DrawElements(GL_TRIANGLES);
    if (flip_progress != 0.0f) {
        sigil_vaos[(page+1)%SPELL_COUNT].DrawElements(GL_TRIANGLES);
        glUniform1f(flip_uniform_index, flip_progress);        
        pages_vao.DrawElements(GL_TRIANGLES);
        if (flip_progress > 0.5f) {
            sigil_vaos[page].DrawElements(GL_TRIANGLES);
        }
    } else {
        sigil_vaos[page].DrawElements(GL_TRIANGLES);
    }    
    glEnable(GL_DEPTH_TEST);
    
};

GLfloat Grimoire::curved_page_verts[PAGE_LOD*6*2];
GLuint Grimoire::curved_page_indeces[PAGE_LOD*2*3];

float pageDepth(float x){
    return sin((pow(x, 0.7f))*PHI*3.14f)*-0.1f;
};

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
        depth = pageDepth(float(i) / float(PAGE_LOD-1));
        
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
    pages_vao.LinkAttrib(pages_vao.vbo, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0); // Position attribute
    pages_vao.LinkAttrib(pages_vao.vbo, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float))); // Texture coord attribute
};
void Grimoire::populateSigilData() {
    int num_verts;
    int w;
    
    for (int s = 0; s < SPELL_COUNT; s++) {        
        num_verts = all_sigils[s].vert_len/2;
        GLfloat sigil_verts[num_verts*6];
        w = 0;
        for (int i = 0; i < num_verts; i++) {
            sigil_verts[w++] = all_sigils[s].verts[i*2];
            sigil_verts[w++] = all_sigils[s].verts[i*2+1];
            sigil_verts[w++] = pageDepth(all_sigils[s].verts[i*2]);
            sigil_verts[w++] = all_sigils[s].verts[i*2]/2.0f;
            sigil_verts[w++] = all_sigils[s].verts[i*2+1]/2.0f;
            sigil_verts[w++] = 2.0f;
        }
        sigil_vaos[s] = VAO(sigil_verts, sizeof(sigil_verts), all_sigils[s].indeces, all_sigils[s].indx_len*sizeof(GLuint));
        sigil_vaos[s].LinkAttrib(sigil_vaos[s].vbo, 0, 3, GL_FLOAT, 6 * sizeof(float), (void*)0);
        sigil_vaos[s].LinkAttrib(sigil_vaos[s].vbo, 1, 3, GL_FLOAT, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    }
}
Grimoire::Grimoire() {
    // Initialize other members...
    populateCurvedPageData();
    populateSigilData();
}
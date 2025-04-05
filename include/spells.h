#ifndef SPELLS_H
#define SPELLS_H

#include "world.h"

const int MAX_SPELLS = 2;
const uint PAGE_LOD = 10;

struct Spell {
    float click_time;
    float release_time;
    float spell_durration = 0.0f;
    float cast_durration = 0.0f;
    float spell_life;
    float cast_life;
    float progress;
    glm::vec3 spell_focus;
    glm::vec3 spell_head;
    glm::vec3 cast_intercepts;
    glm::vec3 cast_player_up;
    int intercept_indeces;
    void (*updateSpellFunction)(PlayerContext*, Spell&);
    void (*startSpellFunction)(PlayerContext*, Spell&);
    Spell();
    Spell(float sd, float cd, 
        void (*f1)(PlayerContext*, Spell&), 
        void (*f2)(PlayerContext*, Spell&));
};

void miningSpell(PlayerContext* player_context, Spell& spell);
void miningSpellCharge(PlayerContext* player_context, Spell& spell);

struct Grimoire
{
    Grimoire();

    Spell all_spells[MAX_SPELLS] = {
        Spell(),
        Spell(0.333f, 1.0f, 
            miningSpell,
            miningSpellCharge)
    };
    
    Spell& active_spell = all_spells[1];

    void updateSpellLife(float time, PlayerContext* context);
    void startSpell(float time, glm::vec3 focus, glm::vec3 head, glm::vec3 player_up, PlayerContext* context);
    void chargeSpell(float time, glm::vec3 focus, glm::vec3 head);    
    
    // Variables/Methods for drawing the Grimoire
    float flip_start, flip_progress = 0.0f, flip_durration = 0.333f;
    bool flip_direction = true;

    void populateCurvedPageData();
    void linkGrimoireVAOs();
    void drawGrimoireVAOs(GLuint flip_uniform_index);
    void flipRight(float time);
    void flipLeft(float time);
    void updateFlip(float time);

    static GLfloat curved_page_verts[PAGE_LOD*6*2];
    static GLuint curved_page_indeces[PAGE_LOD*2*3];
    
    VAO covers_vao, pages_vao;
};

#endif
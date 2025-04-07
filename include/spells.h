#ifndef SPELLS_H
#define SPELLS_H

#define SPELL_COUNT 2

#include "world.h"

const uint PAGE_LOD = 10;

struct Spell {
    float   click_time, 
            release_time,
            spell_durration = 0.0f,
            cast_durration = 0.0f,
            spell_life, 
            cast_life;
    glm::vec3   spell_focus, 
                spell_head,
                cast_intercepts, 
                cast_player_up;
    void (*updateSpellFunction) (PlayerContext*, Spell&);
    void (*startSpellFunction)  (PlayerContext*, Spell&);
    Spell() {};
    Spell(float sd, float cd, 
        void (*f1)(PlayerContext*, Spell&), 
        void (*f2)(PlayerContext*, Spell&)) :
            spell_durration(sd), 
            cast_durration(cd), 
            updateSpellFunction(f1), 
            startSpellFunction(f2) {};
};

void miningSpell(PlayerContext* player_context, Spell& spell);
void buildingSpell(PlayerContext* player_context, Spell& spell);
void emptySpell(PlayerContext* player_context, Spell& spell);

struct Grimoire
{
    Grimoire();

    Spell all_spells[SPELL_COUNT+1] = {
        Spell(),
        Spell(0.333f, 1.0f, 
            miningSpell,
            emptySpell),
        Spell(0.1f, 0.5f,
            buildingSpell,
            emptySpell)
    };
    
    Spell& active_spell = all_spells[1];

    void updateSpellLife(float time, PlayerContext* context);
    void startSpell(float time, glm::vec3 focus, glm::vec3 head, glm::vec3 player_up, PlayerContext* context);
    void chargeSpell(float time, glm::vec3 focus, glm::vec3 head);    
    
    // Variables/Methods for drawing the Grimoire
    float flip_start, flip_progress = 0.0f, flip_durration = 0.333f;
    bool flip_direction = true;

    void drawGrimoireVAOs(GLuint flip_uniform_index);
    void flipRight(float time);
    void flipLeft(float time);
    void updateFlip(float time);
private:
    int page = 0;
    void populateCurvedPageData();
    void populateSigilData();

    static GLfloat curved_page_verts[PAGE_LOD*6*2];
    static GLuint curved_page_indeces[PAGE_LOD*2*3];
    
    VAO covers_vao, pages_vao, sigil_vaos[SPELL_COUNT];
};

#endif
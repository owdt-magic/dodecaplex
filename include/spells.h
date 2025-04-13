#ifndef SPELLS_H
#define SPELLS_H

#define SPELL_COUNT 2

#include "world.h"

const uint PAGE_LOD = 10;

struct Spell {
    float   click_time      = 0.0f,
            release_time    = 0.0f,
            spell_durration = 0.0f,
            cast_durration  = 0.0f,
            spell_life      = 0.0f,
            cast_life       = 0.0f;
    glm::vec3   spell_focus, 
                spell_head,
                cast_intercepts, 
                cast_player_up;
    void (*updateSpellFunction) (PlayerContext*, Spell*);
    void (*startSpellFunction)  (PlayerContext*, Spell*);
    const char* cast_subroutine;
    const char* release_subroutine;
    Spell() {};
    Spell(float sd, float cd, char* cs, char* rs,
        void (*f1)(PlayerContext*, Spell*), 
        void (*f2)(PlayerContext*, Spell*)) :
            spell_durration(sd), 
            cast_durration(cd), 
            cast_subroutine(cs),
            release_subroutine(rs),
            updateSpellFunction(f1), 
            startSpellFunction(f2) {};
    void reset();
};

void miningSpell(   PlayerContext* player_context, Spell* spell);
void buildingSpell( PlayerContext* player_context, Spell* spell);
void emptySpell(    PlayerContext* player_context, Spell* spell);

struct Grimoire {
    Grimoire();

    std::array<Spell, SPELL_COUNT> all_spells = {
        Spell(0.333f, 1.0f, 
            "castMining",
            "releaseMining",
            miningSpell,
            emptySpell),
        Spell(0.1f, 0.5f,
            "releaseMining",
            "releaseMining",
            buildingSpell,
            emptySpell)
    };
    
    Spell* active_spell = &all_spells.at(0);

    void updateSpellLife(   float time, PlayerContext* context);
    void startSpell(        float time, PlayerContext* context);
    void chargeSpell(       float time, PlayerContext* context);
    
    // Variables/Methods for drawing the Grimoire
    float flip_start, flip_progress = 1.0f, flip_durration = 0.45f;
    bool flip_direction = true;

    void drawGrimoireVAOs(GLuint flip_uniform_index);
    void flipRight(float time);
    void flipLeft(float time);
    void updateFlip(float time);
    bool flipping();
private:
    int page = 0;
    void populateCurvedPageData();
    void populateSigilData();

    static GLfloat curved_page_verts[PAGE_LOD*6*2];
    static GLuint curved_page_indeces[PAGE_LOD*2*3];
    
    VAO covers_vao, pages_vao, sigil_vaos[SPELL_COUNT], writing_vao;
};

#endif
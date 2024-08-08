#ifndef SPELLS_H
#define SPELLS_H

#include "world.h"

struct SpellContext
{
    glm::vec3 start; 
    glm::vec3 start_up; 
    glm::vec3 target; 
    int target_index; 
    WorldCell* cell; 
    PlayerContext* context;
    float progress;
    SpellContext(
        glm::vec3 v_1,
        glm::vec3 v_2,
        glm::vec3 v_3,
        int i, 
        WorldCell* wc,
        PlayerContext* pc,
        float p
    ) : start(v_1), start_up(v_2), target(v_3), target_index(i), cell(wc), context(pc), progress(p) {}
};

void teleportA(SpellContext spell_context);
void teleportAStart(SpellContext spell_context);

const int MAX_SPELLS = 2;
const uint PAGE_LOD = 10;

struct Grimoire
{
    Grimoire();
    // {no-spell, teleport_a}
    int active_spell = 1;
    float click_times               [MAX_SPELLS] = {};
    float release_times             [MAX_SPELLS] = {};
    const float spell_durrations    [MAX_SPELLS] = {0.0f, 2.5f};
    const float cast_durrations     [MAX_SPELLS] = {0.0f, 1.0f};
    float spell_life                [MAX_SPELLS] = {};
    float cast_life                 [MAX_SPELLS] = {};
    glm::vec3 spell_focus           [MAX_SPELLS] = {};
    glm::vec3 spell_head            [MAX_SPELLS] = {};
    glm::vec3 cast_intercepts       [MAX_SPELLS] = {};
    glm::vec3 cast_player_up        [MAX_SPELLS] = {};
    int intercept_indeces           [MAX_SPELLS] = {};
    WorldCell* world_cells          [MAX_SPELLS] = {};
    
    void (*updateSpellFunction[MAX_SPELLS])(SpellContext) = {NULL, teleportA};
    void (*startSpellFunction[MAX_SPELLS])(SpellContext) = {NULL, teleportAStart};

    void updateSpellLife(float time, PlayerContext* context);
    void startSpell(float time, glm::vec3 focus, glm::vec3 head, glm::vec3 player_up, InterceptResult intercept_result, PlayerContext* context);
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
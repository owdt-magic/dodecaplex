#ifndef SPELLS_H
#define SPELLS_H

#include "world.h"

void teleportA(float progress, glm::vec3 start, glm::vec3 start_up, glm::vec3 target, int target_index, WorldCell* cell, PlayerContext* context);
void teleportAStart(glm::vec3 start, glm::vec3 start_up, glm::vec3 target, int target_index, WorldCell* cell, PlayerContext* context);

const int MAX_SPELLS = 2;
const uint PAGE_LOD = 10;

struct SpellLog
{
    SpellLog();
    // {no-spell, teleport_a}
    int active_spell = 1;
    float click_times               [MAX_SPELLS] = {};
    float release_times             [MAX_SPELLS] = {};
    const float spell_durrations    [MAX_SPELLS] = {NULL, 2.5f};
    const float cast_durrations     [MAX_SPELLS] = {NULL, 1.0f};
    float spell_life                [MAX_SPELLS] = {};
    float cast_life                 [MAX_SPELLS] = {};
    glm::vec3 spell_focus           [MAX_SPELLS] = {};
    glm::vec3 spell_head            [MAX_SPELLS] = {};
    glm::vec3 cast_intercepts       [MAX_SPELLS] = {};
    glm::vec3 cast_player_up        [MAX_SPELLS] = {};
    int intercept_indeces           [MAX_SPELLS] = {};
    WorldCell* world_cells          [MAX_SPELLS] = {};
    
    void (*updateSpellFunction[MAX_SPELLS])(float, glm::vec3, glm::vec3, glm::vec3, int, WorldCell*, PlayerContext*) = {NULL, teleportA};
    void (*startSpellFunction[MAX_SPELLS])(glm::vec3, glm::vec3, glm::vec3, int, WorldCell*, PlayerContext*) = {NULL, teleportAStart};

    void updateSpellLife(float time, PlayerContext* context) {
        spell_life[active_spell] = 1.0f-std::max(0.0f, std::min(1.0f, (time-release_times[active_spell])/spell_durrations[active_spell]));
        updateSpellFunction[active_spell](
            spell_life          [active_spell], 
            spell_head          [active_spell], 
            cast_player_up      [active_spell],
            cast_intercepts     [active_spell], 
            intercept_indeces   [active_spell], 
            world_cells         [active_spell], context);
    };
    void startSpell(float time, glm::vec3 focus, glm::vec3 head, glm::vec3 player_up, InterceptResult intercept_result, PlayerContext* context) {
        spell_life          [active_spell] = 1.0f;
        release_times       [active_spell] = time;
        spell_focus         [active_spell] = focus;
        spell_head          [active_spell] = head;
        cast_player_up      [active_spell] = player_up;
        world_cells         [active_spell] = intercept_result.cell;
        cast_intercepts     [active_spell] = intercept_result.point;
        intercept_indeces   [active_spell] = intercept_result.index;
        click_times         [active_spell] = NULL;
        startSpellFunction[active_spell](
            spell_head          [active_spell], 
            cast_player_up      [active_spell],
            cast_intercepts     [active_spell], 
            intercept_indeces   [active_spell], 
            world_cells         [active_spell], context);

    };
    void chargeSpell(float time, glm::vec3 focus, glm::vec3 head) {
        cast_life   [active_spell] = std::max(0.0f, std::min(1.0f, (time-click_times[active_spell])/cast_durrations[active_spell]));
        spell_focus [active_spell] = focus;
        spell_head  [active_spell] = head;
    };

    static GLfloat curved_page_verts[PAGE_LOD*6*2];
    static GLuint curved_page_indeces[PAGE_LOD*2*3];
    
    void populateCurvedPageData();
    void linkGrimoireVAOs();
    void drawGrimoireVAOs();

    VAO grimoire_vao;
};

#endif
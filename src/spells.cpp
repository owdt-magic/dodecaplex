#include "spells.h"
#include "sigils.h"
#include <algorithm>
#include <cmath>

using namespace glm;

// ============================================================================
// ISpell Base Class Implementation
// ============================================================================

void ISpell::reset() {
    click_time = 0.0f;
    release_time = 0.0f;
    spell_life = 0.0f;
    cast_life = 0.0f;
}

void ISpell::updateCastLife(float current_time) {
    cast_life = std::clamp((current_time - click_time) / cast_duration, 0.0f, 1.0f);
}

void ISpell::updateSpellLife(float current_time) {
    spell_life = 1.0f - std::clamp((current_time - release_time) / spell_duration, 0.0f, 1.0f);
}

// ============================================================================
// MiningSpell Implementation
// ============================================================================

MiningSpell::MiningSpell() : ISpell(0.333f, 1.0f) {}

void MiningSpell::update(PlayerContext* context) {
    context->elapseShrapnel(spell_life);
}

void MiningSpell::start(PlayerContext* context) {
    // Optional: Add initialization logic when spell is released
}

// ============================================================================
// BuildingSpell Implementation
// ============================================================================

BuildingSpell::BuildingSpell() : ISpell(0.1f, 0.5f) {}

void BuildingSpell::update(PlayerContext* context) {
    context->elapseGrowth(spell_life);
}

void BuildingSpell::start(PlayerContext* context) {
    // Optional: Add initialization logic when spell is released
}

// ============================================================================
// EmptySpell Implementation
// ============================================================================

EmptySpell::EmptySpell() : ISpell(0.0f, 0.0f) {}

void EmptySpell::update(PlayerContext* context) {
    // No-op
}

void EmptySpell::start(PlayerContext* context) {
    // No-op
}

// ============================================================================
// Grimoire Implementation
// ============================================================================

Grimoire::Grimoire() {
    // Initialize spell collection with concrete spell types
    all_spells[0] = std::make_unique<MiningSpell>();
    all_spells[1] = std::make_unique<BuildingSpell>();

    // Set initial active spell
    active_spell = all_spells[0].get();

    // Initialize rendering data
    populateCurvedPageData();
    populateSigilData();
}

void Grimoire::updateSpellLife(float time, PlayerContext* context) {
    active_spell->updateSpellLife(time);
    active_spell->update(context);
}

void Grimoire::startSpell(float time, PlayerContext* context) {
    active_spell->spell_life = 1.0f;
    active_spell->release_time = time;
    active_spell->spell_focus = context->player_location->getFocus();
    active_spell->spell_head = context->player_location->getHead();
    active_spell->cast_player_up = context->player_location->getPUp();
    active_spell->click_time = 0.0f;

    active_spell->start(context);
}

void Grimoire::chargeSpell(float time, PlayerContext* context) {
    active_spell->updateCastLife(time);
    active_spell->spell_focus = context->player_location->getFocus();
    active_spell->spell_head = context->player_location->getHead();
    active_spell->update(context);
}

void Grimoire::flipRight(float time) {
    if (time - flip_start > 0.5f) {
        flip_direction = true;
        flip_start = time;
        flip_progress = -1.0f;

        page = (page == SPELL_COUNT - 1) ? 0 : page + 1;
        active_spell = all_spells[page].get();
        active_spell->reset();
    }
}

void Grimoire::flipLeft(float time) {
    if (time - flip_start > 0.5f) {
        flip_direction = false;
        flip_start = time;
        flip_progress = 1.0f;

        page = (page == 0) ? SPELL_COUNT - 1 : page - 1;
        active_spell = all_spells[page].get();
        active_spell->reset();
    }
}

void Grimoire::updateFlip(float time) {
    static float start = -1.0f;
    static float end = 1.0f;

    flip_progress = mix(start, end, (time - flip_start) / flip_duration);
    flip_progress = std::clamp(flip_progress, start, end);

    if (!flip_direction) {
        flip_progress *= -1.0f;
    }
}

bool Grimoire::flipping() const {
    return std::abs(flip_progress) < 1.0f;
}

const char* Grimoire::getCurrentCastSubroutine() const {
    return active_spell->getCastSubroutine();
}

const char* Grimoire::getCurrentReleaseSubroutine() const {
    return active_spell->getReleaseSubroutine();
}

void Grimoire::drawGrimoireVAOs(GLuint flip_uniform_index) {
    int page_off, flip_page, still_page;

    glDisable(GL_DEPTH_TEST);

    // Draw back pages
    glUniform1f(flip_uniform_index, -1.0f);
    pages_vao.DrawElements(GL_TRIANGLES);
    writing_vao.DrawElements(GL_TRIANGLES);

    // Draw front pages
    glUniform1f(flip_uniform_index, 1.0f);
    pages_vao.DrawElements(GL_TRIANGLES);

    // Determine which page to show
    page_off = (page + 1) % SPELL_COUNT;
    flip_page = flip_direction ? page : page_off;
    still_page = flip_direction ? page_off : page;

    sigil_vaos[still_page].DrawElements(GL_TRIANGLES);

    // Draw flipping page
    glUniform1f(flip_uniform_index, flip_progress);
    pages_vao.DrawElements(GL_TRIANGLES);

    if (flip_progress > 0.0f) {
        sigil_vaos[flip_page].DrawElements(GL_TRIANGLES);
    } else {
        writing_vao.DrawElements(GL_TRIANGLES);
    }

    glEnable(GL_DEPTH_TEST);
}

// ============================================================================
// Page Rendering Helpers
// ============================================================================

GLfloat Grimoire::curved_page_verts[PAGE_LOD*6*2];
GLuint Grimoire::curved_page_indeces[PAGE_LOD*2*3];

static float pageDepth(float x) {
    return std::sin((std::pow(x, 0.7f)) * PHI * 3.14f) * -0.1f;
}

void Grimoire::populateCurvedPageData() {
    static const float upper_bound = 0.0f;
    static const float lower_bound = 1.0f;
    static const float left_bound = 0.0f;
    static const float right_bound = 1.0f;

    static const float texture_top = 0.044f;
    static const float texture_bottom = 0.9531f;
    static const float texture_left = 0.1875f;
    static const float texture_right = 0.8691f;

    int vi = 0, pi = 0;

    for (int i = 0; i < PAGE_LOD; i++) {
        float inter_vert = mix(left_bound, right_bound, float(i) / float(PAGE_LOD - 1));
        float inter_text = mix(texture_left, texture_right, float(i) / float(PAGE_LOD - 1));
        float depth = pageDepth(float(i) / float(PAGE_LOD - 1));

        // Top vertex
        curved_page_verts[vi++] = inter_vert;
        curved_page_verts[vi++] = upper_bound;
        curved_page_verts[vi++] = depth;
        curved_page_verts[vi++] = inter_text;
        curved_page_verts[vi++] = texture_top;
        curved_page_verts[vi++] = 1.0f;

        // Bottom vertex
        curved_page_verts[vi++] = inter_vert;
        curved_page_verts[vi++] = lower_bound;
        curved_page_verts[vi++] = depth;
        curved_page_verts[vi++] = inter_text;
        curved_page_verts[vi++] = texture_bottom;
        curved_page_verts[vi++] = 1.0f;

        if (i != 0) {
            // First triangle
            curved_page_indeces[pi++] = (vi - 12) / 6;
            curved_page_indeces[pi++] = (vi - 18) / 6;
            curved_page_indeces[pi++] = (vi - 24) / 6;

            // Second triangle
            curved_page_indeces[pi++] = (vi - 6) / 6;
            curved_page_indeces[pi++] = (vi - 12) / 6;
            curved_page_indeces[pi++] = (vi - 18) / 6;
        }
    }

    pages_vao = VAO(curved_page_verts, sizeof(curved_page_verts),
                    curved_page_indeces, sizeof(curved_page_indeces));
    pages_vao.LinkVecs({3, 3}, 6);
}

static VAO sigilToVAO(Sigil sigil) {
    int num_verts = sigil.vert_len / 2;
    GLfloat sigil_verts[num_verts * 6];

    int w = 0;
    for (int i = 0; i < num_verts; i++) {
        sigil_verts[w++] = sigil.verts[i * 2];
        sigil_verts[w++] = sigil.verts[i * 2 + 1];
        sigil_verts[w++] = pageDepth(sigil.verts[i * 2]);
        sigil_verts[w++] = sigil.verts[i * 2] / 2.0f;
        sigil_verts[w++] = sigil.verts[i * 2 + 1] / 2.0f;
        sigil_verts[w++] = 2.0f;
    }

    return VAO(sigil_verts, sizeof(sigil_verts),
               sigil.indeces, sigil.indx_len * sizeof(GLuint));
}

void Grimoire::populateSigilData() {
    for (int s = 0; s < SPELL_COUNT; s++) {
        sigil_vaos[s] = sigilToVAO(all_sigils[s]);
        sigil_vaos[s].LinkVecs({3, 3}, 6);
    }

    writing_vao = sigilToVAO(writing);
    writing_vao.LinkVecs({3, 3}, 6);
}

#ifndef SPELLS_H
#define SPELLS_H

#include "world.h"
#include <memory>
#include <array>

const uint PAGE_LOD = 10;

// Forward declaration
class PlayerContext;

// Abstract base class for all spells
class ISpell {
public:
    // Spell state
    float click_time = 0.0f;
    float release_time = 0.0f;
    float spell_duration = 0.0f;
    float cast_duration = 0.0f;
    float spell_life = 0.0f;
    float cast_life = 0.0f;

    glm::vec3 spell_focus;
    glm::vec3 spell_head;
    glm::vec3 cast_intercepts;
    glm::vec3 cast_player_up;

    // Constructor
    ISpell(float spell_dur, float cast_dur)
        : spell_duration(spell_dur), cast_duration(cast_dur) {}

    virtual ~ISpell() = default;

    // Pure virtual methods that subclasses must implement
    virtual void update(PlayerContext* context) = 0;
    virtual void start(PlayerContext* context) = 0;
    virtual const char* getCastSubroutine() const = 0;
    virtual const char* getReleaseSubroutine() const = 0;

    // Common functionality
    void reset();
    void updateCastLife(float current_time);
    void updateSpellLife(float current_time);
};

// Mining spell - destroys surfaces with shrapnel
class MiningSpell : public ISpell {
public:
    MiningSpell();

    void update(PlayerContext* context) override;
    void start(PlayerContext* context) override;
    const char* getCastSubroutine() const override { return "castMining"; }
    const char* getReleaseSubroutine() const override { return "releaseMining"; }
};

// Building spell - creates/removes cells
class BuildingSpell : public ISpell {
public:
    BuildingSpell();

    void update(PlayerContext* context) override;
    void start(PlayerContext* context) override;
    const char* getCastSubroutine() const override { return "releaseMining"; }
    const char* getReleaseSubroutine() const override { return "releaseMining"; }
};

// Empty spell - passive/no effect
class EmptySpell : public ISpell {
public:
    EmptySpell();

    void update(PlayerContext* context) override;
    void start(PlayerContext* context) override;
    const char* getCastSubroutine() const override { return "emptySpell"; }
    const char* getReleaseSubroutine() const override { return "emptySpell"; }
};

// Grimoire manages spells and UI
class Grimoire {
public:
    Grimoire();
    ~Grimoire() = default;

    // Spell management
    void updateSpellLife(float time, PlayerContext* context);
    void startSpell(float time, PlayerContext* context);
    void chargeSpell(float time, PlayerContext* context);

    // Page navigation
    void flipRight(float time);
    void flipLeft(float time);
    void updateFlip(float time);
    bool flipping() const;

    // Rendering
    void drawGrimoireVAOs(GLuint flip_uniform_index);

    // Get current spell's shader subroutines
    const char* getCurrentCastSubroutine() const;
    const char* getCurrentReleaseSubroutine() const;

    // Access current spell state
    float getSpellLife() const { return active_spell->spell_life; }
    float getCastLife() const { return active_spell->cast_life; }
    glm::vec3 getSpellFocus() const { return active_spell->spell_focus; }
    glm::vec3 getSpellHead() const { return active_spell->spell_head; }

private:
    // Spell collection - using unique_ptr for polymorphism
    static constexpr size_t SPELL_COUNT = 2;
    std::array<std::unique_ptr<ISpell>, SPELL_COUNT> all_spells;
    ISpell* active_spell = nullptr;
    int page = 0;

    // Page flip animation state
    float flip_start = 0.0f;
    float flip_progress = 1.0f;
    float flip_duration = 0.45f;
    bool flip_direction = true;

    // Rendering data
    void populateCurvedPageData();
    void populateSigilData();

    static GLfloat curved_page_verts[PAGE_LOD*6*2];
    static GLuint curved_page_indeces[PAGE_LOD*2*3];

    VAO covers_vao;
    VAO pages_vao;
    VAO sigil_vaos[SPELL_COUNT];
    VAO writing_vao;
};

#endif // SPELLS_REFACTORED_H

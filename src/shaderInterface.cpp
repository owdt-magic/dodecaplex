#include "graphicsPipe.h"

void ShaderInterface::compile() {
    // Base implementation - should be overridden
}

void ShaderInterface::render() {
    // Base implementation - should be overridden
}

GamePatterns::GamePatterns(CLAs c, Uniforms* w) : ShaderInterface(c, w) {
    world_shader = ShaderProgram(
        SHADER_DIR "/world.vert",
        SHADER_DIR "/prune.geom", 
        SHADER_DIR "/spell_dodecaplex.frag", false);
    fx_shader = ShaderProgram(
        SHADER_DIR "/shrapnel.vert",
        SHADER_DIR "/prune.geom",
        SHADER_DIR "/spell_dodecaplex.frag", false);
    gui_shader = ShaderProgram(
        SHADER_DIR "/book.vert",
        SHADER_DIR "/book.frag", false);
    
    player_context.initializeMapData();
    player_context.populateDodecaplexVAO();

    glGenBuffers(1, &U_GLOBAL);
    glBindBuffer(GL_UNIFORM_BUFFER, U_GLOBAL);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, U_GLOBAL);
}

void GamePatterns::compile() {
    world_shader.Load();
    world_shader.Activate();
    
    texture_library.linkPentagonLibrary(world_shader.ID);

    U_RESOLUTION  = glGetUniformLocation(world_shader.ID, "u_resolution");
    U_MOUSE       = glGetUniformLocation(world_shader.ID, "u_mouse");
    U_SCROLL      = glGetUniformLocation(world_shader.ID, "u_scroll");
    U_TIME        = glGetUniformLocation(world_shader.ID, "u_time");
    U_CAST_LIFE   = glGetUniformLocation(world_shader.ID, "CAST_LIFE");
    U_SPELL_LIFE  = glGetUniformLocation(world_shader.ID, "SPELL_LIFE");            
    U_SPELL_FOCUS = glGetUniformLocation(world_shader.ID, "SPELL_FOCUS");
    U_SPELL_HEAD  = glGetUniformLocation(world_shader.ID, "SPELL_HEAD");

    fx_shader.Load();
    fx_shader.Activate();

    texture_library.linkPentagonLibrary(fx_shader.ID); 
    S_SPELL_LIFE  = glGetUniformLocation(fx_shader.ID, "SPELL_LIFE");            
    
    gui_shader.Load();
    gui_shader.Activate();
    texture_library.linkGrimoireLibrary(gui_shader.ID);

    U_FLIP_PROGRESS = glGetUniformLocation(gui_shader.ID, "u_flip_progress");
    U_TIME_BOOK     = glGetUniformLocation(gui_shader.ID, "u_time");

    window_uniforms->player_context = &player_context;
}

void GamePatterns::render() {
    float time = glfwGetTime();

    accountCameraControls(window_uniforms, cam);

    glBindBuffer(GL_UNIFORM_BUFFER, U_GLOBAL);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,                 sizeof(glm::mat4), &(cam.Projection)[0][0]);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &(cam.Model)[0][0]);

    world_shader.Activate();

    glUniform2f(U_RESOLUTION,   window_uniforms->windWidth,
                                window_uniforms->windHeight);
    glUniform2f(U_MOUSE,        window_uniforms->mouseX,
                                window_uniforms->mouseY);
    glUniform1f(U_SCROLL,       window_uniforms->scroll);
    glUniform1f(U_TIME, time);

    subroutine_index = getSpellSubroutine(window_uniforms, grimoire, world_shader.ID);

    glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &subroutine_index);

    glUniform1f(U_CAST_LIFE,    grimoire.active_spell->cast_life);
    glUniform1f(U_SPELL_LIFE,   grimoire.active_spell->spell_life);
    glUniform3f(U_SPELL_FOCUS,  grimoire.active_spell->spell_focus.x,
                                grimoire.active_spell->spell_focus.y,
                                grimoire.active_spell->spell_focus.z);
    glUniform3f(U_SPELL_HEAD,   grimoire.active_spell->spell_head.x,
                                grimoire.active_spell->spell_head.y,
                                grimoire.active_spell->spell_head.z);
    player_context.drawMainVAO();
    
    fx_shader.Activate();
    glUniform1f(S_SPELL_LIFE,   grimoire.active_spell->spell_life);
    player_context.drawShrapnelVAOs();

    gui_shader.Activate();
    glUniform1f(U_TIME_BOOK, time);
    grimoire.drawGrimoireVAOs(U_FLIP_PROGRESS);
}

SpinPatterns::SpinPatterns(CLAs c, Uniforms* w) : ShaderInterface(c, w) {
    spin_shader = ShaderProgram(
        SHADER_DIR "/spin.vert",
        SHADER_DIR "/spin.geom",
        SHADER_DIR "/spin.frag", false);
    
    player_context.initializeMapData();
    player_context.populateDodecaplexVAO(RhombusPattern(WebType::DOUBLE_STAR, false), true);

    glGenBuffers(1, &U_GLOBAL);
    glBindBuffer(GL_UNIFORM_BUFFER, U_GLOBAL);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, U_GLOBAL);
}

void SpinPatterns::compile() {
    spin_shader.Load();
    spin_shader.Activate();

    U_RESOLUTION  = glGetUniformLocation(spin_shader.ID, "u_resolution");
    U_MOUSE       = glGetUniformLocation(spin_shader.ID, "u_mouse");
    U_SCROLL      = glGetUniformLocation(spin_shader.ID, "u_scroll");
    U_TIME        = glGetUniformLocation(spin_shader.ID, "u_time");
    U_BANDS       = glGetUniformLocation(spin_shader.ID, "u_audio_bands");
    U_BRIGHTNESS  = glGetUniformLocation(spin_shader.ID, "u_brightness");
    U_SCALE       = glGetUniformLocation(spin_shader.ID, "u_scale");
    U_HUESHIFT    = glGetUniformLocation(spin_shader.ID, "u_hueShift");
    U_VIGNETTE    = glGetUniformLocation(spin_shader.ID, "u_vignette");
    U_LINE_PX     = glGetUniformLocation(spin_shader.ID, "u_linePx");
    U_LINE_FADE   = glGetUniformLocation(spin_shader.ID, "u_lineFade");
    U_SHATTER     = glGetUniformLocation(spin_shader.ID, "u_shatter");

    window_uniforms->player_context = &player_context;
}

void SpinPatterns::render() {
    float time = glfwGetTime();
    
    accountSpin(window_uniforms, cam,   shared_uniforms.data->speed, 
                                        shared_uniforms.data->fov, 
                                        shared_uniforms.data->scroll);

    glBindBuffer(GL_UNIFORM_BUFFER, U_GLOBAL);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,                 sizeof(glm::mat4), &(cam.Projection)[0][0]);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &(cam.Model)[0][0]);

    spin_shader.Activate();

    glUniform2f(U_RESOLUTION,   window_uniforms->windWidth,
                                window_uniforms->windHeight);
    glUniform2f(U_MOUSE,        window_uniforms->mouseX,
                                window_uniforms->mouseY);
    glUniform1f(U_SCROLL,       window_uniforms->scroll);
    glUniform1f(U_TIME, time);

    glUniform4f(U_BANDS,    shared_uniforms.data->audio_bands[0],
                            shared_uniforms.data->audio_bands[1],
                            shared_uniforms.data->audio_bands[2],
                            shared_uniforms.data->audio_bands[3]);

    glUniform1f(U_BRIGHTNESS, shared_uniforms.data->brightness);
    glUniform1f(U_SCALE,      shared_uniforms.data->scale);
    glUniform1f(U_HUESHIFT,   shared_uniforms.data->hueShift);
    glUniform1f(U_VIGNETTE,   shared_uniforms.data->vignette);
    glUniform1f(U_LINE_PX,    shared_uniforms.data->linePx);
    glUniform1f(U_LINE_FADE,  shared_uniforms.data->lineFade);
    glUniform1f(U_SHATTER,    shared_uniforms.data->shatter);

    player_context.drawMainVAO();
}

// FragPatterns implementation
FragPatterns::FragPatterns(CLAs c, Uniforms* w) : ShaderInterface(c, w) {
    std::string fragShaderPath = clas.shaderPath.empty()
        ? std::string(FRAG_SHADER_DIR) + "/purple-vortex.frag"
        : clas.shaderPath;
    frag_shader = ShaderProgram(FRAG_SHADER_DIR "/rect.vert", fragShaderPath, false);
    fullscreenQuad = rasterPipeVAO();
}

void FragPatterns::compile() {
    frag_shader.Load();
    frag_shader.Activate();
    
    U_RESOLUTION  = glGetUniformLocation(frag_shader.ID, "u_resolution");
    U_MOUSE       = glGetUniformLocation(frag_shader.ID, "u_mouse");
    U_SCROLL      = glGetUniformLocation(frag_shader.ID, "u_scroll");
    U_TIME        = glGetUniformLocation(frag_shader.ID, "u_time");
    U_SCALE       = glGetUniformLocation(frag_shader.ID, "u_scale");
    U_BRIGHTNESS  = glGetUniformLocation(frag_shader.ID, "u_brightness");
    U_SPEED       = glGetUniformLocation(frag_shader.ID, "u_speed");
    U_FOV         = glGetUniformLocation(frag_shader.ID, "u_fov");
    U_HUESHIFT    = glGetUniformLocation(frag_shader.ID, "u_hueShift");
    U_AUDIO_BANDS = glGetUniformLocation(frag_shader.ID, "u_audio_bands");
}

void FragPatterns::render() {
    float time = glfwGetTime();
    
    frag_shader.Activate();
    
    glUniform2f(U_RESOLUTION,   window_uniforms->windWidth,
                                window_uniforms->windHeight);
    glUniform2f(U_MOUSE,        window_uniforms->mouseX,
                                window_uniforms->mouseY);
    glUniform1f(U_SCROLL,       window_uniforms->scroll);
    glUniform1f(U_TIME, time);

    glUniform4f(U_AUDIO_BANDS,  shared_uniforms.data->audio_bands[0],
                                shared_uniforms.data->audio_bands[1],
                                shared_uniforms.data->audio_bands[2],
                                shared_uniforms.data->audio_bands[3]);

    glUniform1f(U_SCALE,      shared_uniforms.data->scale);
    glUniform1f(U_BRIGHTNESS, shared_uniforms.data->brightness);
    glUniform1f(U_SPEED,      shared_uniforms.data->speed);
    glUniform1f(U_FOV,        shared_uniforms.data->fov);
    glUniform1f(U_HUESHIFT,   shared_uniforms.data->hueShift);

    fullscreenQuad.DrawElements(GL_TRIANGLES);
}
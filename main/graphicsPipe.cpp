#include "audio.h"
#include "config.h"
#include "gameWindow.h"
#include "shaderClass.h"
#include "bufferObjects.h"
#include "models.h"
#include <chrono>
#include <thread>
#include "debug.h"
#include "cla.h"
#include "sharedUniforms.h"

enum PipeType {
    GAME,
    SPIN,
    FRAGMENT
}

struct ShaderInterface {
    CLAS clas;
    SharedUniforms sus;
    Uniforms* wus;
    virtual void complie();
    virtual void draw();
}

struct GamePatterns : public ShaderInterface {    
    ShaderProgram world_shader, fx_shader, gui_shader;
    PlayerContext player_context;
    TextureLibrary texture_library;
    Grimoire grimoire;
    CameraInfo cam;

    GLuint  U_RESOLUTION, U_MOUSE, U_SCROLL, U_TIME,
            U_SPELL_LIFE, U_CAST_LIFE, U_SPELL_FOCUS, U_SPELL_HEAD,
            U_FLIP_PROGRESS, U_TIME_BOOK;
    GLuint  S_SPELL_LIFE;
    GLuint subroutine_index;
    GLuint U_GLOBAL;

    GamePatterns(CLAs c, SharedUniforms s, Uniforms* w) clas(c) sus(s) wus(w){
        world_shader(
            SHADER_DIR "/world.vert", \
            SHADER_DIR "/prune.geom", \
            SHADER_DIR "/world.frag", false);
        fx_shader(
            SHADER_DIR "/shrapnel.vert", \
            SHADER_DIR "/prune.geom", \
            SHADER_DIR "/spell_dodecaplex.frag", false);
        gui_shader(
            SHADER_DIR "/book.vert",\
            SHADER_DIR "/book.frag", false);        
        
        player_context.initializeMapData();
        player_context.populateDodecaplexVAO();

        glGenBuffers(1, &U_GLOBAL);
        glBindBuffer(GL_UNIFORM_BUFFER, U_GLOBAL);
        glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, U_GLOBAL);
    };
    void complie() {
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
        U_TIME_BOOK     = glGetUniformLocationg(gui_shader.ID, "u_time");
    };
    void draw() {
        accountCameraControls(wus, cam);

        glBufferSubData(GL_UNIFORM_BUFFER, 0,                 sizeof(glm::mat4), &(cam.Projection)[0][0]);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &(cam.Model)[0][0]);

        world_shader.Activate();

        glUniform2f(U_RESOLUTION,   wus->windWidth, \
                                    wus->windHeight);
        glUniform2f(U_MOUSE,        wus->mouseX, \
                                    wus->mouseY);
        glUniform1f(U_SCROLL,       wus->scroll);
        glUniform1f(U_TIME, time);

        subroutine_index = getSpellSubroutine(wus, grimoire, world_shader.ID);
        
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
        
        shrapnel_shader.Activate();
        glUniform1f(S_SPELL_LIFE,   grimoire.active_spell->spell_life);
        player_context.drawShrapnelVAOs();

        book_shader.Activate();
        glUniform1f(U_TIME_BOOK, time);
        grimoire.drawGrimoireVAOs(U_FLIP_PROGRESS);

    };
}

struct SpinPatterns : public ShaderInterface {
    ShaderProgram spin_shader;
    PlayerContext player_context;
    SpinPatterns(CLAs c, SharedUniforms s, Uniforms* w) clas(c) sus(s) wus(w){
        spin_shader(     
            SHADER_DIR "/world.vert", \
            SHADER_DIR "/prune.geom", \
            SHADER_DIR "/spin.frag", false);
        
        player_context.initializeMapData();
        player_context.populateDodecaplexVAO();
    };
    void compile() {
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
    };
    void draw() {
        accountSpin(wus, cam,  sus.data->speed, 
                               sus.data->fov, 
                               sus.data->scroll);

        glBufferSubData(GL_UNIFORM_BUFFER, 0,                 sizeof(glm::mat4), &(cam.Projection)[0][0]);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &(cam.Model)[0][0]);

        world_shader.Activate();

        glUniform2f(U_RESOLUTION,   wus->windWidth, \
                                    wus->windHeight);
        glUniform2f(U_MOUSE,        wus->mouseX, \
                                    wus->mouseY);
        glUniform1f(U_SCROLL,       wus->scroll);
        glUniform1f(U_TIME, time);

        // Use shared audio bands from select.cpp
        glUniform4f(U_BANDS,    sus.data->audio_bands[0],
                                sus.data->audio_bands[1],
                                sus.data->audio_bands[2],
                                sus.data->audio_bands[3]);

        glUniform1f(U_BRIGHTNESS, sus.data->brightness);
        glUniform1f(U_SCALE,      sus.data->scale);
        glUniform1f(U_HUESHIFT,   sus.data->hueShift);
        glUniform1f(U_VIGNETTE,   sus.data->vignette);

        player_context.drawMainVAO();

    };
}

struct FragPatterns : public ShaderInterface {
    ShaderProgram frag_shader;
    VAO fullscreenQuad;
    FragPatterns(CLAs c, SharedUniforms s, Uniforms* w) clas(c) sus(s) wus(s){
        std::string fragShaderPath = clas.shaderPath.empty()
        ? std::string(FRAG_SHADER_DIR) + "/purple-vortex.frag"
        : clas.shaderPath;
        frag_shader( FRAG_SHADER_DIR "/rect.vert", fragShaderPath, false);
        fullscreenQuad = rasterPipeVAO();
    };
    void compile() {
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
    };
    void draw() {
        glUniform2f(U_RESOLUTION,   wus->windWidth, \
                                    wus->windHeight);
        glUniform2f(U_MOUSE,        wus->mouseX, \
                                    wus->mouseY);
        glUniform1f(U_SCROLL,       wus->scroll);
        glUniform1f(U_TIME, time);

        glUniform1f(U_SCALE,      sus.data->scale);
        glUniform1f(U_BRIGHTNESS, sus.data->brightness);
        glUniform1f(U_SPEED,      sus.data->speed);
        glUniform1f(U_FOV,        sus.data->fov);
        glUniform1f(U_HUESHIFT,   sus.data->hueShift);
        glUniform4f(U_AUDIO_BANDS,  sus.data->audio_bands[0],
                                    sus.data->audio_bands[1],
                                    sus.data->audio_bands[2],
                                    sus.data->audio_bands[3]);

        fullscreenQuad.DrawElements(GL_TRIANGLES);
    };
        
}

struct GraphicsPipe {
    CLAs clas;
    Uniforms* window_uniforms;
    SharedUniforms shared_uniforms;
    ShaderInterface* shader_interface;
    
    static float time, previousTime;
    static int frameCount;
    PipeType type;
    const char * window_name;
    GLFWwindow* window;
    
    GraphicsPipe(PipeType t, CLAs c) : type(t) clas(c) {
        switch (type){
            case PipeType::GAME:
            window_name = "DODECAPLEX";
            break;
            case PipeType::SPIN:
            window_name = "SPINNING DODECAPLEX";
            break;
            case PipeType::FRAGMENT:
            window_name = "FRAGMENT SHADER";
            break;
        }
    };
    void initHere(GLFWwindow* w) window(w) {
        window_uniforms = getUniforms(window);
        switch (type) {
            case PipeType::SPIN {
                shader_interface = (ShaderInterface*) &SpinPatterns(shared_uniforms(false), window_uniforms, clas);
                break;   
            }
            case PipeType::FRAGMENT {
                shader_interface = (ShaderInterface*) &FragPatterns(shared_uniforms(false), window_uniforms, clas);
                break;
            }
            case PipeType::GAME {
                shader_interface = (ShaderInterface*) &GamePatterns(shared_uniforms(false), window_uniforms, clas);
                break;
            }
        }
        previousTime = glfwGetTime();
        frameCount = 0;
    };
    void initWindowed() {
        initHere(initializeWindow(1024, 1024, window_name, 
                                    clas.fullscreen, clas.monitorIndex));
    };
    void establishShaders() {
        shader_interface.compile();

        window_uniforms->last_time         = glfwGetTime();
        window_uniforms->loading           = false;
        window_uniforms->player_context    = &player_context;
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
    };
    void renderNextFrame() {    
        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        shader_interface.render();

        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        glfwPollEvents();
        window_uniforms->last_time = time;
    };
}

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
};

struct ShaderInterface {
    CLAs clas;
    Uniforms* window_uniforms;
    CameraInfo cam;
    ShaderInterface(CLAs c, Uniforms* w) : 
        clas(c), window_uniforms(w) {};
    virtual ~ShaderInterface() = default;
    virtual void compile() = 0;
    virtual void render() = 0;
};

struct GamePatterns : public ShaderInterface {    
    ShaderProgram world_shader, fx_shader, gui_shader;
    PlayerContext player_context;
    TextureLibrary texture_library;
    Grimoire grimoire;

    GLuint  U_RESOLUTION, U_MOUSE, U_SCROLL, U_TIME,
            U_SPELL_LIFE, U_CAST_LIFE, U_SPELL_FOCUS, U_SPELL_HEAD,
            U_FLIP_PROGRESS, U_TIME_BOOK;
    GLuint  S_SPELL_LIFE;
    GLuint subroutine_index;
    GLuint U_GLOBAL;

    GamePatterns(CLAs c, Uniforms* w);
    void compile() override;
    void render() override;
};

struct SpinPatterns : public ShaderInterface {
    ShaderProgram spin_shader;
    PlayerContext player_context;
    
    GLuint U_RESOLUTION, U_MOUSE, U_SCROLL, U_TIME, U_BANDS, U_SCALE, U_BRIGHTNESS, U_HUESHIFT, U_VIGNETTE;
    GLuint U_GLOBAL;
    SharedUniforms shared_uniforms = SharedUniforms(false);

    SpinPatterns(CLAs c, Uniforms* w);
    void compile() override;
    void render() override;
};

struct FragPatterns : public ShaderInterface {
    ShaderProgram frag_shader;
    VAO fullscreenQuad;
    
    GLuint  U_RESOLUTION, U_MOUSE, U_SCROLL, U_TIME,
            U_SCALE, U_BRIGHTNESS, U_SPEED, U_FOV, U_HUESHIFT, U_AUDIO_BANDS;
    SharedUniforms shared_uniforms = SharedUniforms(false);

    FragPatterns(CLAs c, Uniforms* w);
    void compile() override;
    void render() override;
};

struct GraphicsPipe {
    CLAs clas;
    Uniforms* window_uniforms;
    ShaderInterface* shader_interface;
    
    static float time, previousTime;
    static int frameCount;
    PipeType type;
    const char* window_name;
    GLFWwindow* window;
    
    GraphicsPipe(PipeType t, CLAs c);
    ~GraphicsPipe();
    void initHere(GLFWwindow* w);
    void initWindowed();
    void establishShaders();
    void renderNextFrame(bool swapBuffers = true);
};

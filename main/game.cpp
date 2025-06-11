#include "config.h"
#include "gameWindow.h"
#include "shaderClass.h"
#include "bufferObjects.h"
#include "models.h"
#include <chrono>
#include <thread>
#include "debug.h"
#include "cla.h"

int main(int argc, char** argv) {
    CLAs clas = parse(argc, argv);    

    GLFWwindow* window = initializeWindow(1024, 1024, "DODECAPLEX");

    ShaderProgram world_shader(     SHADER_DIR "/world.vert", \
                                    SHADER_DIR "/prune.geom", \
                                    SHADER_DIR "/spell_dodecaplex.frag", false);
    ShaderProgram shrapnel_shader(  SHADER_DIR "/shrapnel.vert", \
                                    SHADER_DIR "/prune.geom", \
                                    SHADER_DIR "/spell_dodecaplex.frag", false);
    ShaderProgram book_shader(      SHADER_DIR "/book.vert",\
                                    SHADER_DIR "/book.frag", false);
    
    PlayerContext player_context;
    player_context.initializeMapData();

    Grimoire grimoire;    

    Uniforms* uniforms = getUniforms(window);
    
    float time;
    GLuint  U_RESOLUTION, U_MOUSE, U_SCROLL, U_TIME,
            U_SPELL_LIFE, U_CAST_LIFE, U_SPELL_FOCUS, U_SPELL_HEAD,
            U_FLIP_PROGRESS, U_TIME_BOOK;
    GLuint  S_SPELL_LIFE;
    GLuint subroutine_index;
    CameraInfo cam;

    TextureLibrary texture_library;
    player_context.populateDodecaplexVAO();
    
    GLuint U_GLOBAL;
    glGenBuffers(1, &U_GLOBAL);
    glBindBuffer(GL_UNIFORM_BUFFER, U_GLOBAL);
    glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, U_GLOBAL);

    while (!glfwWindowShouldClose(window)) {
        if (uniforms->loading) {
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

            shrapnel_shader.Load();
            shrapnel_shader.Activate();

            texture_library.linkPentagonLibrary(shrapnel_shader.ID); 
                // TODO: This, doesn't seem explicitly necessary??
            S_SPELL_LIFE  = glGetUniformLocation(shrapnel_shader.ID, "SPELL_LIFE");            
            
            book_shader.Load();
            book_shader.Activate();
            texture_library.linkGrimoireLibrary(book_shader.ID);

            U_FLIP_PROGRESS = glGetUniformLocation(book_shader.ID, "u_flip_progress");
            U_TIME_BOOK     = glGetUniformLocation(book_shader.ID, "u_time");

            uniforms->last_time         = glfwGetTime();
            uniforms->loading           = false;
            uniforms->player_context    = &player_context;
            
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
        }        
        time = glfwGetTime();
        uniforms->this_time = time;

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        accountCameraControls(uniforms, cam);

        glBufferSubData(GL_UNIFORM_BUFFER, 0,                 sizeof(glm::mat4), &(cam.Projection)[0][0]);
        glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &(cam.Model)[0][0]);

        world_shader.Activate();

        glUniform2f(U_RESOLUTION,   uniforms->windWidth, \
                                    uniforms->windHeight);
        glUniform2f(U_MOUSE,        uniforms->mouseX, \
                                    uniforms->mouseY);
        glUniform1f(U_SCROLL,       uniforms->scroll);
        glUniform1f(U_TIME, time);

        subroutine_index = getSpellSubroutine(uniforms, grimoire, world_shader.ID);
        
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

        glfwSwapInterval(1);
        glfwSwapBuffers(window);
        glfwPollEvents();
        uniforms->last_time = time;
        //checkGLError("At end of loop...");
    }
    return 0;
}
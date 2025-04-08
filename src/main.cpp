#include "config.h"
#include "window.h"
#include "shaderClass.h"
#include "bufferObjects.h"
#include "models.h"
#include <chrono>
#include <thread>
#include "debug.h"

#define DRAW_BOOK

int main() {
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
    GLuint  U_RESOLUTION, U_MOUSE, U_SCROLL, U_TIME, U_CAMERA, U_WORLD,
            U_SPELL_LIFE, U_CAST_LIFE, U_SPELL_FOCUS, U_SPELL_HEAD,
            U_FLIP_PROGRESS, U_CAMERA_BOOK, U_TIME_BOOK;      
    GLuint  S_CAMERA, S_WORLD, S_SPELL_LIFE;
    GLuint subroutine_index;
    CameraInfo cam;

    TextureLibrary texture_library;
    player_context.populateDodecaplexVAO();
    
    while (!glfwWindowShouldClose(window)) {
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (uniforms->loading) {
            world_shader.Load();
            world_shader.Activate();
            
            texture_library.linkPentagonLibrary(world_shader.ID);

            U_RESOLUTION  = glGetUniformLocation(world_shader.ID, "u_resolution");
            U_MOUSE       = glGetUniformLocation(world_shader.ID, "u_mouse");
            U_SCROLL      = glGetUniformLocation(world_shader.ID, "u_scroll");
            U_TIME        = glGetUniformLocation(world_shader.ID, "u_time");
            U_CAMERA      = glGetUniformLocation(world_shader.ID, "CAMERA");
            U_WORLD       = glGetUniformLocation(world_shader.ID, "WORLD");
            U_CAST_LIFE   = glGetUniformLocation(world_shader.ID, "CAST_LIFE");
            U_SPELL_LIFE  = glGetUniformLocation(world_shader.ID, "SPELL_LIFE");            
            U_SPELL_FOCUS = glGetUniformLocation(world_shader.ID, "SPELL_FOCUS");
            U_SPELL_HEAD  = glGetUniformLocation(world_shader.ID, "SPELL_HEAD");

            shrapnel_shader.Load();
            shrapnel_shader.Activate();

            texture_library.linkPentagonLibrary(shrapnel_shader.ID); 
                // TODO: This, doesn't seem explicitly necessary??
            S_CAMERA      = glGetUniformLocation(shrapnel_shader.ID, "CAMERA");
            S_WORLD       = glGetUniformLocation(shrapnel_shader.ID, "WORLD");
            S_SPELL_LIFE  = glGetUniformLocation(shrapnel_shader.ID, "SPELL_LIFE");            
            
            #ifdef DRAW_BOOK
            book_shader.Load();
            book_shader.Activate();
            texture_library.linkGrimoireLibrary(book_shader.ID);

            U_FLIP_PROGRESS = glGetUniformLocation(book_shader.ID, "u_flip_progress");
            U_CAMERA_BOOK   = glGetUniformLocation(book_shader.ID, "CAMERA");
            U_TIME_BOOK     = glGetUniformLocation(book_shader.ID, "u_time");
            #endif

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

        accountCameraControls(uniforms, cam);

        glUniformMatrix4fv(U_CAMERA, 1, GL_FALSE, &(cam.Projection)[0][0]);
        glUniformMatrix4fv(U_WORLD,  1, GL_FALSE, &(cam.Model)[0][0]);
        
        player_context.drawMainVAO();
        
        shrapnel_shader.Activate();
        glUniform1f(S_SPELL_LIFE,   grimoire.active_spell->spell_life);
                
        glUniformMatrix4fv(S_CAMERA, 1, GL_FALSE, &(cam.Projection)[0][0]);
        glUniformMatrix4fv(S_WORLD,  1, GL_FALSE, &(cam.Model)[0][0]);

        player_context.drawShrapnelVAOs();

        #ifdef DRAW_BOOK
        book_shader.Activate();
        
        glUniformMatrix4fv(U_CAMERA_BOOK, 1, GL_FALSE, &(cam.Projection)[0][0]);
        glUniform1f(U_TIME_BOOK, time);

        grimoire.drawGrimoireVAOs(U_FLIP_PROGRESS);
        #endif

        glfwSwapBuffers(window);
        glfwPollEvents();
        uniforms->last_time = time;
        //checkGLError("At end of loop...");
    }
    return 0;
}
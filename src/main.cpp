#include "config.h"
#include "window.h"
#include "shaderClass.h"
#include "bufferObjects.h"
#include "models.h"
#include <chrono>
#include <thread>
#include "debug.h"

int main() {
    GLFWwindow* window = initializeWindow(768, 768, "DODECAPLEX");

    ShaderProgram world_shader( SHADER_DIR "/displacement_dodecaplex.vert", \
                                SHADER_DIR "/spell_dodecaplex.frag", false);
    ShaderProgram book_shader(  SHADER_DIR "/book.vert",\
                                SHADER_DIR "/book.frag", false);
    
    
    PlayerContext player_context;
    //player_context.linkPlayerCellVAOs();
    player_context.linkDodecaplexVAOs();
    
    Grimoire grimoire;    
    grimoire.linkGrimoireVAOs();

    Uniforms* uniforms = getUniforms(window);
    
    float time;
    GLuint  U_RESOLUTION, U_MOUSE, U_SCROLL, U_TIME, U_HEAD, U_CAMERA, U_WORLD,
            U_MATRIX_0, U_MATRIX_1, U_MATRIX_2, U_AXIS_0, U_AXIS_1, U_AXIS_2,
            U_SPELL_LIFE, U_CAST_LIFE, U_SPELL_FOCUS, U_SPELL_HEAD,
            U_FLIP_PROGRESS, U_CAMERA_BOOK;            
    GLuint subroutine_index;
    CameraInfo cam;

    TextureLibrary texture_library;

    while (!glfwWindowShouldClose(window)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (uniforms->loading) {
            world_shader.Load();
            world_shader.Activate();
            texture_library.linkPentagonLibrary(world_shader.ID);

            U_RESOLUTION  = glGetUniformLocation(world_shader.ID, "u_resolution");
            U_MOUSE       = glGetUniformLocation(world_shader.ID, "u_mouse");
            U_SCROLL      = glGetUniformLocation(world_shader.ID, "u_scroll");
            U_TIME        = glGetUniformLocation(world_shader.ID, "u_time");
            U_HEAD        = glGetUniformLocation(world_shader.ID, "HEAD");
            U_CAMERA      = glGetUniformLocation(world_shader.ID, "CAMERA");
            U_WORLD       = glGetUniformLocation(world_shader.ID, "WORLD");
            U_CAST_LIFE   = glGetUniformLocation(world_shader.ID, "CAST_LIFE");
            U_SPELL_LIFE  = glGetUniformLocation(world_shader.ID, "SPELL_LIFE");            
            U_SPELL_FOCUS = glGetUniformLocation(world_shader.ID, "SPELL_FOCUS");
            U_SPELL_HEAD  = glGetUniformLocation(world_shader.ID, "SPELL_HEAD");

            U_MATRIX_0    = glGetUniformLocation(world_shader.ID, "MATRIX_0");
            U_MATRIX_1    = glGetUniformLocation(world_shader.ID, "MATRIX_1");
            U_MATRIX_2    = glGetUniformLocation(world_shader.ID, "MATRIX_2");
            U_AXIS_0      = glGetUniformLocation(world_shader.ID, "AXIS_0");
            U_AXIS_1      = glGetUniformLocation(world_shader.ID, "AXIS_1");
            U_AXIS_2      = glGetUniformLocation(world_shader.ID, "AXIS_2");
            
            book_shader.Load();
            book_shader.Activate();
            texture_library.linkGrimoireLibrary(book_shader.ID);

            U_FLIP_PROGRESS = glGetUniformLocation(book_shader.ID, "u_flip_progress");
            U_CAMERA_BOOK   = glGetUniformLocation(book_shader.ID, "CAMERA");

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

        glUniform1f(U_CAST_LIFE,    grimoire.cast_life[grimoire.active_spell]);
        glUniform1f(U_SPELL_LIFE,   grimoire.spell_life[grimoire.active_spell]);
        glUniform3f(U_SPELL_FOCUS,  grimoire.spell_focus[grimoire.active_spell].x,
                                    grimoire.spell_focus[grimoire.active_spell].y,
                                    grimoire.spell_focus[grimoire.active_spell].z);
        glUniform3f(U_SPELL_HEAD,   grimoire.spell_head[grimoire.active_spell].x,
                                    grimoire.spell_head[grimoire.active_spell].y,
                                    grimoire.spell_head[grimoire.active_spell].z);

        accountCameraControls(uniforms, cam);

        glUniformMatrix4fv(U_CAMERA, 1, GL_FALSE, &(cam.Projection*cam.View)[0][0]);
        glUniformMatrix4fv(U_WORLD,  1, GL_FALSE, &(cam.Model)[0][0]);
        glUniform3f(U_HEAD, cam.Location.x, cam.Location.y, cam.Location.z);

        glUniformMatrix4fv(U_MATRIX_0, 1, GL_FALSE, &(cam.M0)[0][0]);
        glUniformMatrix4fv(U_MATRIX_1, 1, GL_FALSE, &(cam.M1)[0][0]);
        glUniformMatrix4fv(U_MATRIX_2, 1, GL_FALSE, &(cam.M2)[0][0]);
        glUniform4f(U_AXIS_0, cam.A0.x, cam.A0.y, cam.A0.z, cam.A0.w);
        glUniform4f(U_AXIS_1, cam.A1.x, cam.A1.y, cam.A1.z, cam.A1.w);
        glUniform4f(U_AXIS_2, cam.A2.x, cam.A2.y, cam.A2.z, cam.A2.w);
        
        player_context.drawAllVAOs();

        book_shader.Activate();
        
        glUniformMatrix4fv(U_CAMERA_BOOK, 1, GL_FALSE, &(cam.Projection)[0][0]);

        grimoire.drawGrimoireVAOs(U_FLIP_PROGRESS);

        glfwSwapBuffers(window);
        glfwPollEvents();
        uniforms->last_time = time;
        checkGLError("At end of loop...");
    }
    return 0;
}
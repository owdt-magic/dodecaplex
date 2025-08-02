#include "graphicsPipe.h"

int main(int argc, char** argv) {
    CLAs clas = parse(argc, argv);    

    GraphicsPipe pipe(PipeType::GAME, clas);
    pipe.initWindowed();    

    while (!glfwWindowShouldClose(pipe.window)) {
        if (pipe.window_uniforms->loading) {
            pipe.establishShaders();
        }
        pipe.renderNextFrame();
    }

    return 0;
}
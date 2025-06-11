#include <string>
#include "cla.h"

CLAs parse(int argc, char** argv){
    CLAs out;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--input" && i + 1 < argc) {
            out.audioIndex = std::stoi(argv[i + 1]);
        } else if (std::string(argv[i]) == "--fullscreen") {
            out.fullscreen = true;
        } else if (std::string(argv[i]) == "--monitor" && i + 1 < argc) {
            out.monitorIndex = std::stoi(argv[++i]);
        } else if (std::string(argv[i]) == "--shader" && i + 1 < argc) {
            out.shaderPath = argv[++i];
        }
    }
    return out;
};
struct CLAs {
    bool fullscreen     = false;
    int monitorIndex    = 0;
    int audioIndex      = 0;
    std::string shaderPath = "";
};

CLAs parse(int argc, char** argv);

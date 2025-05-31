struct CLAs {
    bool fullscreen     = false;
    int monitorIndex    = 0;
    int audioIndex      = 0;
};

CLAs parse(int argc, char** argv);


#pragma once

struct UniformStructure {
    float scale;
    float brightness;
    float speed;
    float warp;
};

struct SharedUniforms {
    UniformStructure* data;
    int fd;
    SharedUniforms(){
        fd = open("/tmp/uniforms.dat", O_RDWR | O_CREAT, 0666);
        ftruncate(fd, sizeof(UniformStructure));
        data = (UniformStructure*)mmap(NULL, sizeof(UniformStructure), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    };
    ~SharedUniforms(){
        munmap(data, sizeof(UniformStructure));
        close(fd);
        unlink("/tmp/uniforms.dat");
    };
};


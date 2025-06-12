#pragma once
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

struct UniformStructure {
    float scale;
    float brightness;
    float speed;
    float fov;
    UniformStructure() :    scale(0.0f),
                            brightness(1.0f),
                            speed(1.0f),
                            fov(150.0f) {};
};

struct SharedUniforms {
    UniformStructure* data;
    const char* loc = "/tmp/uniforms.dat";
    int fd;
    SharedUniforms(bool writeable){
        writeable ? openRW() : openR();
    };
private:
    void write(){
        *data = UniformStructure();
    };
    void openRW(){
        fd = open(loc, O_RDWR | O_CREAT, 0666);
        ftruncate(fd, sizeof(UniformStructure));
        data = (UniformStructure*)mmap(NULL, sizeof(UniformStructure), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        write();
    };
    void openR(){
        fd = open(loc, O_RDONLY);
        if (fd != -1) {
        void* mmap_ptr = mmap(NULL, sizeof(float), PROT_READ, MAP_SHARED, fd, 0);
        if (mmap_ptr != MAP_FAILED) {
            std::cout << "Default shared uniforms" << std::endl;
            data = (UniformStructure*) mmap_ptr;
        } else {            
            close(fd);
            *data = UniformStructure();
        }
    }    
    };
public:
    ~SharedUniforms(){
        munmap(data, sizeof(UniformStructure));
        close(fd);
        unlink(loc);
    };
};


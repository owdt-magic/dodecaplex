#pragma once
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define BAND_COUNT 4
#define PARAM_COUNT 6

struct UniformStructure {
    float scale;
    float brightness;
    float speed;
    float fov;
    float hueShift;    
    float scroll;
    
    float volume; 
    float audio_bands[BAND_COUNT];  // FFT band amplitudes
    int audio_routing[BAND_COUNT];  // Bit flags for parameter routing (bit 0=scale, 1=brightness, 2=speed, 3=fov, 4=hueShift)
    UniformStructure() :    scale(0.0f),
                            brightness(1.0f),
                            speed(1.0f),
                            fov(150.0f),
                            hueShift(0.0f),
                            volume(1.0f),
                            scroll(0.0f) {
        for(int i = 0; i < BAND_COUNT; i++) {
            audio_bands[i] = 0.0f;
            audio_routing[i] = 0;  // No routing by default
        }
    };
};

struct UniformMeta {
    float* value;
    float min, max;
    const char * name;
    UniformMeta(float* v, float x, float y, const char* n) : 
        value(v), min(x), max(y), name(n) {};
};

struct SharedUniforms {
    UniformStructure* data;
    const char* loc = "/tmp/uniforms.dat";
    int fd;
    bool should_unlink;

    UniformMeta metadata[PARAM_COUNT];
    
    SharedUniforms(bool writeable)
        : should_unlink(writeable),
          metadata{
              UniformMeta(nullptr, 0.0f, 1.0f, "Scale"),
              UniformMeta(nullptr, 0.0f, 2.0f, "Brightness"),
              UniformMeta(nullptr, 0.0f, 2.0f, "Speed"),
              UniformMeta(nullptr, 0.0f, 180.0f, "FOV"),
              UniformMeta(nullptr, 0.0f, 360.0f, "Hue Shift"),
              UniformMeta(nullptr, -100.0f, 100.0f, "Scroll")
          }
    {
        writeable ? openRW() : openR();
        // Now update the value pointers after data is initialized
        metadata[0].value = &data->scale;
        metadata[1].value = &data->brightness;
        metadata[2].value = &data->speed;
        metadata[3].value = &data->fov;
        metadata[4].value = &data->hueShift;
        metadata[5].value = &data->scroll;
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
        if (should_unlink) {
            unlink(loc);
        }
    };
    void ApplyRouting(std::atomic<float>* g_bandAmplitudes){
        for(int i = 0; i < BAND_COUNT; i++) {
            data->audio_bands[i] = (g_bandAmplitudes[i])*data->volume;
            
            int routing       = data->audio_routing[i];
            float audio_value = data->audio_bands[i];
            for (int i = 0; i < PARAM_COUNT; i++){
                if (routing & (1 << i)) *metadata[i].value = audio_value*metadata[i].max + metadata[i].min;
            }
        }
    }
};


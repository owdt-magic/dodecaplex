#pragma once
#include <vector>
#include "miniaudio/miniaudio.h"

constexpr int SAMPLE_RATE = 48000;
constexpr int BUFFER_SIZE = 2048;

ma_device_id select_input_device(ma_context* context);

std::vector<std::string> GetInputDeviceNames(ma_context* context);

struct AudioNest {
    ma_device device;
    ma_context context;
    static std::array<float, BUFFER_SIZE> g_audioBuffer;
    std::atomic<float> g_bandAmplitudes[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    int deviceIndex;
    
    AudioNest(int index) : deviceIndex(index) {
        startAudioDevice();
    };
    
    static void data_callback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
        static size_t g_writeHead = 0;
        const float* in = (const float*)input;        

        for (ma_uint32 i = 0; i < frameCount && g_writeHead < BUFFER_SIZE; ++i) {
            g_audioBuffer[g_writeHead++] = in[i]; // mono
        }
        if (g_writeHead >= BUFFER_SIZE) {
            g_writeHead = 0;
        }
        
    }
    void startAudioDevice();
    void changeAudioDevice(int index);
    void processFFT();
};
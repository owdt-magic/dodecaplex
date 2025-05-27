#include <vector>
#include "miniaudio/miniaudio.h"

constexpr int SAMPLE_RATE = 48000;

ma_device_id select_input_device(ma_context* context);

void processFFT(const std::vector<float>& audioFrame, std::atomic<float>* g_bandAmplitudes);


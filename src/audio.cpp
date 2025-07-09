#include <iostream>
#include <complex>
#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"

std::array<float, BUFFER_SIZE> AudioNest::g_audioBuffer = {};

ma_device_id select_input_device(ma_context* context) {
    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;

    if (ma_context_get_devices(context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        std::cerr << "Failed to enumerate audio devices.\n";
        return {};
    }

    std::cout << "\nAvailable Input Devices:\n";
    for (ma_uint32 i = 0; i < captureCount; ++i) {
        std::cout << i << ": " << pCaptureInfos[i].name << "\n";
    }

    int selection = -1;
    while (selection < 0 || selection >= static_cast<int>(captureCount)) {
        std::cout << "Select an input device by number: ";
        std::cin >> selection;
    }

    return pCaptureInfos[selection].id;
}

// Recursive FFT function using std::complex
void fft(std::vector<std::complex<float>>& data) {
    size_t N = data.size();
    if (N <= 1) return;

    std::vector<std::complex<float>> even(N / 2), odd(N / 2);
    for (size_t i = 0; i < N / 2; ++i) {
        even[i] = data[i * 2];
        odd[i] = data[i * 2 + 1];
    }

    fft(even);
    fft(odd);

    for (size_t k = 0; k < N / 2; ++k) {
        float angle = -2.0f * M_PI * k / float(N);
        std::complex<float> twiddle(std::cos(angle), std::sin(angle));
        data[k] = even[k] + twiddle * odd[k];
        data[k + N / 2] = even[k] - twiddle * odd[k];
    }
}

// Main function to process FFT and extract 4 bands
void AudioNest::processFFT() {
    size_t N = BUFFER_SIZE;
    std::vector<std::complex<float>> data(N);
    for (size_t i = 0; i < N; ++i) {
        data[i] = std::complex<float>(g_audioBuffer[i], 0.0f);
    }

    fft(data); // In-place

    // Compute magnitudes
    std::vector<float> magnitudes(N / 2);
    for (size_t i = 0; i < N / 2; ++i) {
        magnitudes[i] = std::abs(data[i]);
    }

    // Frequency band boundaries in Hz
    float bandHz[5] = { 60.0f, 250.0f, 1000.0f, 4000.0f, SAMPLE_RATE / 2.0f };
    size_t bandIndices[5];
    for (int i = 0; i < 5; ++i) {
        bandIndices[i] = static_cast<size_t>(bandHz[i] * N / SAMPLE_RATE);
        if (bandIndices[i] >= N / 2) bandIndices[i] = N / 2 - 1;
    }

    // Compute average magnitude per band
    for (int b = 0; b < 4; ++b) {
        float sum = 0.0f;
        size_t start = bandIndices[b];
        size_t end = bandIndices[b + 1];
        for (size_t i = start; i < end; ++i) {
            sum += magnitudes[i];
        }
        g_bandAmplitudes[b] = (end > start) ? (sum / (end - start)) : 0.0f;
    }
}

std::vector<std::string> GetInputDeviceNames(ma_context* context) {
    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;
    std::vector<std::string> names;

    if (ma_context_get_devices(context, &pPlaybackInfos, &playbackCount, &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        return names;
    }

    for (ma_uint32 i = 0; i < captureCount; ++i) {
        names.emplace_back(pCaptureInfos[i].name);
    }
    return names;
}

void AudioNest::startAudioDevice(){
    ma_result result = ma_context_init(NULL, 0, NULL, &context);
    if (result != MA_SUCCESS) {
        std::cerr << "Failed to initialize miniaudio context.\n";
        return;
    }

    ma_device_info* pPlaybackInfos;
    ma_uint32 playbackCount;
    ma_device_info* pCaptureInfos;
    ma_uint32 captureCount;

    if (ma_context_get_devices(&context, &pPlaybackInfos, &playbackCount,
                            &pCaptureInfos, &captureCount) != MA_SUCCESS) {
        std::cerr << "Failed to enumerate audio devices.\n";
        return;
    }

    ma_device_id selectedInputDevice = pCaptureInfos[deviceIndex].id;

    ma_device_config deviceConfig = ma_device_config_init(ma_device_type_capture);
    deviceConfig.capture.pDeviceID = &selectedInputDevice;
    deviceConfig.capture.format   = ma_format_f32;
    deviceConfig.capture.channels = 1;
    deviceConfig.sampleRate       = SAMPLE_RATE;
    deviceConfig.dataCallback     = AudioNest::data_callback;

    if (ma_device_init(&context, &deviceConfig, &device) != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio device.\n";
        return;
    }
    ma_device_start(&device);    
};

void AudioNest::changeAudioDevice(int index) {
    // Stop the current device
    ma_device_stop(&device);
    ma_device_uninit(&device);
    
    // Update the device index
    deviceIndex = index;
    
    // Restart with the new device
    startAudioDevice();
}

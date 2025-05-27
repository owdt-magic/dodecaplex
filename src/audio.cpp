#include <iostream>
#include <complex>
#define MINIAUDIO_IMPLEMENTATION
#include "audio.h"

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
void processFFT(const std::vector<float>& audioFrame, std::atomic<float>* g_bandAmplitudes) {
    size_t N = audioFrame.size();
    std::vector<std::complex<float>> data(N);
    for (size_t i = 0; i < N; ++i) {
        data[i] = std::complex<float>(audioFrame[i], 0.0f);
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


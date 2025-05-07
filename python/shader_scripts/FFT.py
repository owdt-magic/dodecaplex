import numpy as np
import sounddevice as sd
import soundfile as sf
from scipy.signal import stft
import threading
import time
import moderngl_window as mglw
from Default import UniformImporter

class FFTShader(UniformImporter):
    vertex_shader = 'vertex.glsl'
    fragment_shader = 'fft_shader.frag'
    wav_path = 'audio/tts_output.wav'

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.sample_rate = 44100
        self.update_rate = 1 / 60  # 60 fps
        self.band_count = 4
        self.current_frame = 0
        self.current_sample = 0

        self.audio_data, self.sample_rate = sf.read(self.wav_path, always_2d=True)
        self.compute_fft_frames()

        self.total_samples = len(self.audio_data)
        self.total_duration = self.total_samples / self.sample_rate

        self.audio_thread = threading.Thread(target=self.play_audio_loop, daemon=True)
        self.audio_thread.start()

    def compute_fft_frames(self):
        f, t, Zxx = stft(self.audio_data[:, 0], fs=self.sample_rate, nperseg=1024)
        bands = [(20, 200), (200, 800), (800, 3000), (3000, 12000)]
        self.band_energies = self.extract_band_energies(Zxx, f, bands)
        self.norm_energies = self.normalize_energies(self.band_energies)
        self.total_frames = self.norm_energies.shape[1]
        self.start_time = time.time()

    def extract_band_energies(self, Zxx, f, bands):
        magnitude = np.abs(Zxx)
        energies = []
        for (start, end) in bands:
            band_indices = np.where((f >= start) & (f < end))[0]
            band_energy = magnitude[band_indices].mean(axis=0)
            energies.append(band_energy)
        return np.array(energies)

    def normalize_energies(self, energies):
        return np.log1p(energies / (np.max(energies, axis=1, keepdims=True) + 1e-6))

    def play_audio_loop(self):
        self.playback_start_time = time.time()

        def callback(outdata, frames, time_info, status):
            end = self.current_sample + frames
            chunk = self.audio_data[self.current_sample:end]
            if len(chunk) < frames:
                # Loop the audio
                remaining = frames - len(chunk)
                chunk = np.concatenate((chunk, self.audio_data[:remaining]))
                self.playback_start_time = time.time()  # reset clock
                self.current_sample = remaining
            else:
                self.current_sample += frames
            outdata[:] = chunk.reshape(-1, 1) if len(chunk.shape) == 1 else chunk

        self.stream = sd.OutputStream(
            samplerate=self.sample_rate,
            channels=self.audio_data.shape[1] if len(self.audio_data.shape) > 1 else 1,
            callback=callback
        )
        self.stream.start()

    def get_audio_progress(self):
        elapsed = time.time() - self.playback_start_time
        progress = (elapsed % self.total_duration) / self.total_duration
        return progress

    def on_render(self, time, frame_time):
        super().on_render(time, frame_time)

        frame_idx = int(self.total_frames * self.get_audio_progress())
        if frame_idx >= self.total_frames:
            frame_idx = int(frame_idx%self.total_frames)

        for i in range(self.band_count):
            uniform_name = f'u_band{i}'
            if uniform_name in self.program:
                self.program[uniform_name] = float(self.norm_energies[i, frame_idx])
            else:
                print(f"MISSING UNIFORM: {uniform_name}")



if __name__ == "__main__":
    mglw.run_window_config(FFTShader)
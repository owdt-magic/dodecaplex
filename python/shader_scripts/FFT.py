import os
import glob
import numpy as np
import sounddevice as sd
import soundfile as sf
from scipy.signal import stft
import threading
import time
import moderngl_window as mglw
from Default import UniformImporter
import random


class FFTShader(UniformImporter):
    vertex_shader = 'vertex.glsl'
    fragment_shader = 'fft_shader.frag'

    def __init__(self, **kwargs):
        super().__init__(**kwargs)
        self.band_count = 4
        self.audio_data = None
        self.sample_rate = None
        self.norm_energies = None
        self.total_frames = 0
        self.total_duration = 0
        self.current_sample = 0
        self.playback_start_time = None
        self.stream = None
        self.audio_thread = None
        self.audio_files = sorted(glob.glob("./audio/*.mp3"))[:9]
        self.playing = False
        self.finished = True  # True if no file playing

    def load_audio(self, file_path):
        print(f"Loading: {file_path}")
        self.audio_data, self.sample_rate = sf.read(file_path, always_2d=True)
        self.total_samples = len(self.audio_data)
        self.total_duration = self.total_samples / self.sample_rate
        self.compute_fft_frames()
        self.current_sample = 0

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

    def stop_audio(self):
        if self.stream:
            try:
                self.stream.stop()
                self.stream.close()
            except Exception as e:
                print("Error stopping stream:", e)
            self.stream = None
        self.playing = False
        self.finished = True

    def play_audio_file_once(self, index):
        if index >= len(self.audio_files):
            print("No file for index", index)
            return

        self.stop_audio()
        self.load_audio(self.audio_files[index])

        def audio_thread_fn():
            self.playing = True
            self.finished = False
            self.playback_start_time = time.time()
            self.current_sample = 0

            def callback(outdata, frames, time_info, status):
                start = self.current_sample
                end = start + frames
                if end >= self.total_samples:
                    chunk = self.audio_data[start:self.total_samples]
                    pad = frames - len(chunk)
                    outdata[:len(chunk)] = chunk
                    outdata[len(chunk):] = np.zeros((pad, self.audio_data.shape[1]))
                    raise sd.CallbackStop()
                else:
                    chunk = self.audio_data[start:end]
                    outdata[:] = chunk
                    self.current_sample += frames

            try:
                with sd.OutputStream(
                    samplerate=self.sample_rate,
                    channels=self.audio_data.shape[1],
                    callback=callback,
                    blocksize=1024,
                    latency='low'
                ) as stream:
                    self.stream = stream
                    stream.start()
                    while stream.active:
                        time.sleep(0.01)
            except Exception as e:
                print("Stream error:", e)
            finally:
                self.stop_audio()

        self.audio_thread = threading.Thread(target=audio_thread_fn, daemon=True)
        self.audio_thread.start()

    def get_audio_progress(self):
        if not self.playing or not self.playback_start_time:
            return 0.0
        elapsed = time.time() - self.playback_start_time
        return min(elapsed / self.total_duration, 1.0)

    def on_key_event(self, key, action, modifiers):
        super().on_key_event(key, action, modifiers)
        if action == self.wnd.keys.ACTION_PRESS:
            index = key - self.wnd.keys.NUMBER_1  # maps key 1 to index 0
            if 0 <= index < len(self.audio_files):
                print(f"Playing: {self.audio_files[index]}")
                self.play_audio_file_once(index)                


    def on_render(self, time, frame_time):
        super().on_render(time, frame_time)

        if self.norm_energies is None or self.finished:
            return  # Nothing to render if no file or playback done

        frame_idx = int(self.total_frames * self.get_audio_progress())
        frame_idx = min(frame_idx, self.total_frames - 1)

        for i in range(self.band_count):
            uniform_name = f'u_band{i}'
            if uniform_name in self.program:
                self.program[uniform_name] = float(self.norm_energies[i, frame_idx])


class FFTShaderAuto(FFTShader):
    def __init__(self, interval=30, **kwargs):
        super().__init__(**kwargs)
        self.interval = interval
        self.auto_thread = None
        self.auto_stop = False
        print(f"[FFTShaderAuto] Initialized with {len(self.audio_files)} audio files.")
        self.start_auto_play()

    def start_auto_play(self):
        if not self.audio_files:
            print("[FFTShaderAuto] No audio files found. Auto-play will not start.")
            return
        self.auto_stop = False
        self.schedule_next_play()
        print("[FFTShaderAuto] Auto-play started.")

    def stop_auto_play(self):
        self.auto_stop = True
        if self.auto_thread:
            self.auto_thread.cancel()
            self.auto_thread = None
        print("[FFTShaderAuto] Auto-play stopped.")

    def schedule_next_play(self):
        if self.auto_stop:
            return
        self.auto_thread = threading.Timer(self.interval, self.auto_play)
        self.auto_thread.start()
        print(f"[FFTShaderAuto] Next audio will play in {self.interval} seconds.")

    def auto_play(self):
        if self.auto_stop or not self.audio_files:
            return
        index = random.randint(0, len(self.audio_files) - 1)
        print(f"[FFTShaderAuto] [AUTO] Playing: {self.audio_files[index]}")
        self.play_audio_file_once(index)
        self.schedule_next_play()

    def on_key_event(self, key, action, modifiers):
        # Ignore all key input
        pass

    def on_close(self):
        self.stop_auto_play()
        if hasattr(super(), 'on_close'):
            super().on_close()

if __name__ == "__main__":
    mglw.run_window_config(FFTShaderAuto)

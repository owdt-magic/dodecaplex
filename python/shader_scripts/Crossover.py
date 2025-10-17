import pyaudio
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

import sounddevice as sd

def select_input_device():
    devices = sd.query_devices()
    input_devices = [d for d in devices if d['max_input_channels'] > 0]

    print("Available input devices:")
    for i, dev in enumerate(input_devices):
        print(f"{i}: {dev['name']}")

    idx = int(input("Select the index of your system audio input device: "))
    sd.default.device = (input_devices[idx]['name'], None)
    print(f"Selected input device: {input_devices[idx]['name']}")


import numpy as np
import sounddevice as sd
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from scipy.signal import butter, lfilter

# Audio setup
samplerate = 44100
blocksize = 1024
channels = 1

# Frequency band definitions (in Hz)
bands = [(80, 100), (100, 600), (600, 2000), (2000, 8000)]

# Envelope smoothing (between 0.90 and 0.999 for smooth, low-latency response)
envelope_smoothing = 0.01

# Design bandpass filters
def make_bandpass_filter(lowcut, highcut, fs, order=4):
    nyq = fs * 0.5
    b, a = butter(order, [lowcut / nyq, highcut / nyq], btype='band')
    return b, a

filters = [make_bandpass_filter(low, high, samplerate) for (low, high) in bands]

# Envelope state
envelopes = [0.0] * len(bands)

def envelope(signal, prev, smoothing):
    """
    Simple envelope follower using exponential smoothing of absolute value
    """
    return smoothing * prev + (1 - smoothing) * np.mean(np.abs(signal))

# Audio callback
def audio_callback(indata, frames, time, status):
    global envelopes
    audio = indata[:, 0]  # mono

    for i, (b, a) in enumerate(filters):
        filtered = lfilter(b, a, audio)
        envelopes[i] = envelope(filtered, envelopes[i], envelope_smoothing)

# Visualization setup
fig, ax = plt.subplots()
bars = ax.bar(range(len(bands)), [0]*len(bands), color='cyan')
ax.set_ylim(0, .08)
ax.set_xlim(-0.5, len(bands)-0.5)
ax.set_xticks(range(len(bands)))
ax.set_xticklabels([f"{low}-{high}Hz" for (low, high) in bands])
ax.set_ylabel("Amplitude")
ax.set_title("Real-Time Frequency Band Amplitudes (No FFT)")

def update(frame):
    for i, bar in enumerate(bars):
        bar.set_height(envelopes[i])
    return bars

ani = FuncAnimation(fig, update, interval=.01, blit=False)
select_input_device()


# Start audio stream
stream = sd.InputStream(callback=audio_callback, samplerate=samplerate,
                        channels=channels, blocksize=blocksize)
with stream:
    plt.show()


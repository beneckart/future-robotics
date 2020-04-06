import numpy as np
import time
import matplotlib.pyplot as plt
import pyaudio
import simpleaudio as sa

A_STROBE_FREQ = 19200
REF_TONE_FREQ = 18200
REF_PANNING_FREQ = 30.0
R_FREQ = 18700
G_FREQ = 19200
B_FREQ = 19700
BYTES_PER_SAMPLE = 2

A_FREQ = 440
C_SHARP_FREQ = A_FREQ * 2 ** (4 / 12)
E_FREQ = A_FREQ * 2 ** (7 / 12)

SAMPLE_RATE = 44100  # 44100 samples per second
MAX_INT16 = 2**15 - 1
L_IX = 0
R_IX = 1
N_CHANNELS = 2
MAX_VOL = 1.0
ROUNDING_PARAM_DEFAULT = 0.1
FLASHING_FREQ = 1

def get_empty_buffer(duration):
    n_samples = int(SAMPLE_RATE * duration)
    return np.zeros((n_samples, N_CHANNELS))

def t_buffer(duration):
    n_samples = int(SAMPLE_RATE * duration)
    return np.linspace(0, duration, n_samples, False)

def sine_wave(duration, frequency, phase = 0.0):
    t = t_buffer(duration)
    return np.sin(2 * np.pi * frequency * t + phase)

def rounded_square_wave(duration, frequency, phase = 0.0, rounding_param = ROUNDING_PARAM_DEFAULT):
    t = t_buffer(duration)
    return np.arctan(np.sin(2.0 * np.pi * t * frequency + phase) / rounding_param)

def sine_tone(frequency_l, frequency_r, vol_l, vol_r, duration):
    tone = get_empty_buffer(duration)
    tone[:,L_IX] = vol_l * sine_wave(duration, frequency_l)
    tone[:,R_IX] = vol_r * sine_wave(duration, frequency_r)
    return tone

def spectrastrobe_tone(duration, panning_freq = REF_PANNING_FREQ, tone_freq = REF_TONE_FREQ):
    base_tone = sine_tone(tone_freq, tone_freq, MAX_VOL, MAX_VOL, duration)
    # Create a square clipping function
    clip_mask = get_empty_buffer(duration)
    clip_mask[:,0] = np.maximum(0.0, rounded_square_wave(duration, panning_freq))
    clip_mask[:,1] = np.maximum(0.0, rounded_square_wave(duration, panning_freq, np.pi))
    return base_tone * clip_mask    

def plot_tone(tone, xlim = (0.0, 2.0)):
    n_samples = tone.shape[0]
    t = t_buffer(n_samples / SAMPLE_RATE)
    plt.plot(t, tone[:,L_IX], 'g-')
    plt.plot(t, tone[:,R_IX], 'b-')
    plt.xlim(xlim)
    plt.show()

def play_tone(tone):
    # Transform float to int16
    audio = tone * MAX_INT16 / np.max(np.abs(tone))
    audio = audio.astype(np.int16)
    # Start playback
    play_obj = sa.play_buffer(audio, N_CHANNELS, BYTES_PER_SAMPLE, SAMPLE_RATE)
    # Wait for playback to finish before exiting
    play_obj.wait_done()

duration = 10.0
#toneAE = sine_tone(A_FREQ, E_FREQ, MAX_VOL, MAX_VOL, 2)
ref_tone = spectrastrobe_tone(duration)
gb_tone = sine_tone(A_FREQ, E_FREQ, MAX_VOL, MAX_VOL, duration)
flashing_mask = sine_wave(duration, FLASHING_FREQ)**2
gb_tone[:,0] *= flashing_mask
gb_tone[:,1] *= flashing_mask
#plot_tone(ref_tone, (0.0, 0.5))
#plot_tone(gb_tone)
# play_tone(ref_tone + gb_tone)
# play_tone(gb_tone)
# time.sleep(2.0)


# instantiate PyAudio (1)
p = pyaudio.PyAudio()

a_tone = sine_wave(duration, A_FREQ)
a_tone *= flashing_mask
tone = gb_tone
audio = tone * MAX_INT16 / np.max(np.abs(tone))
audio = audio.astype(np.int16)
i = 0
# define callback (2)
def callback(in_data, frame_count, time_info, status):
    global i
    audio_chunk = audio[i : i + frame_count]
    i = i + frame_count
    return (audio_chunk, pyaudio.paContinue)

# open stream using callback (3)
stream = p.open(format=pyaudio.paInt16,
                channels=2,
                rate=SAMPLE_RATE,
                output=True,
                stream_callback=callback)

# start the stream (4)
stream.start_stream()

# wait for stream to finish (5)
while stream.is_active():
    time.sleep(0.1)

# stop stream (6)
stream.stop_stream()
stream.close()

# close PyAudio (7)
p.terminate()

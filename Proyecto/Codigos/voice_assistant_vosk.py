import sounddevice as sd
import queue
import json
from vosk import Model, KaldiRecognizer

# --- Configuracion ---
model = Model("/home/diegofer/vosk_models/model_en")
samplerate = 16000
recognizer = KaldiRecognizer(model, samplerate)
audio_q = queue.Queue()

is_active = False  # Si ya se dijo "entrance"

# --- Callback de audio ---
def callback(indata, frames, time, status):
    if status:
        print(f"Stream status: {status}", flush=True)
    try:
        audio_q.put_nowait(bytes(indata))
    except queue.Full:
        print("Audio queue is full!", flush=True)

# --- Funciones auxiliares ---
def reset_recognizer():
    global recognizer
    recognizer = KaldiRecognizer(model, samplerate)

def clear_audio_queue():
    while not audio_q.empty():
        audio_q.get()

# --- Escuchar ---
def listen():
    global is_active
    print("Say 'entrance' to activate voice control...")
    try:
        with sd.RawInputStream(samplerate=samplerate, blocksize=4000, dtype='int16', channels=1, callback=callback):
            while True:
                try:
                    data = audio_q.get(timeout=1)  # Wait max 1 second
                except queue.Empty:
                    continue  # No data, continue loop

                if recognizer.AcceptWaveform(data):
                    result = json.loads(recognizer.Result())
                    text = result.get("text", "").lower()
                    #print(f"Recognized: {text}")

                    if not text:
                        continue

                    if not is_active:
                        if "entrance" in text:
                            is_active = True
                            print("Activated! Say 'on' to turn ON the LED or 'down' to turn OFF the LED")
                    else:
                        if "on" in text:
                            print("LED ON")
                            #is_active = False
                            clear_audio_queue()
                            reset_recognizer()
                        elif "down" in text:
                            print("LED OFF")
                            #is_active = False
                            clear_audio_queue()
                            reset_recognizer()
                        elif "exit" in text:
                            print("Exiting.")
                            break
    except Exception as e:
        print(f"Error in audio stream: {e}")

if __name__ == "__main__":
    listen()

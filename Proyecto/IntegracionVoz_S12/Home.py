import subprocess, json, numpy as np, os
from vosk import Model, KaldiRecognizer

# --- Configuracion ---
model = Model("/home/root/vosk_models/model_en") 
rate_hw = 32000        # Frecuencia de SPH0645
rate_vosk = 16000      # Frecuencia de Vosk
# Limitar las palabras que se reconocen
recognizer = KaldiRecognizer(model, rate_vosk, '["entrance", "on", "down", "exit"]') 
is_active = False   #No escucha hasta detectar Entrance

# --- Ajustes para microfono ---
cmd = [
    "arecord",
    "-D", "hw:0,0",      # SPH0645
    "-f", "S32_LE",      
    "-r", str(rate_hw),  # 32 kHz
    "-c", "1",           # Mono
    "-q",                # Silencioso
    "-t", "raw"          
]
# Grabar audio en RT y mandarlo a py a procesar
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=0)

print("Entrance para asistente de voz")

# --- Normalizar audio ---
def normalize_audio(data_32):
    data_f = data_32.astype(np.float32)
    rms = np.sqrt(np.mean(data_f ** 2))
    if rms < 1:
        rms = 1
    gain = 20000.0 / rms
    data_norm = np.clip(data_f * gain, -32768, 32767)
    return data_norm.astype(np.int16)

while True:
    data = proc.stdout.read(4000 * 4)   # Lee 4000 muestras * 4 bytes directamente desde el microfono
    if not data:
        continue

    # Conversion y muestreo
    data_32 = np.frombuffer(data, dtype=np.int32)
    data_16 = normalize_audio(data_32)
    data_16_resampled = data_16[::2]  # 32k ? 16k

    # Vosk
    if recognizer.AcceptWaveform(data_16_resampled.tobytes()):
        text = json.loads(recognizer.Result()).get("text", "").lower()
        if not text:
            continue

        print(f"Reconocido: {text}")

        if not is_active:
            if "entrance" in text:
                is_active = True
                print("'on' para encender el LED o 'down' para apagarlo.")
        else:
            if "on" in text:
                print("LED ON")
                os.system("/home/root/led_on.sh")
            elif "down" in text:
                print("LED OFF")
                os.system("/home/root/led_off.sh")
            elif "exit" in text:        # Cerrar asistente de voz hasta nuevo entrance
                print("Saliendo del asistente de voz.")
                is_active = False

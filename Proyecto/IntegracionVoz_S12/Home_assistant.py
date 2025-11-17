import subprocess, json, numpy as np, os
from vosk import Model, KaldiRecognizer

# --- Configuración ---
model = Model("/home/root/vosk_models/model_en")  # Ruta del modelo
rate_hw = 32000        # Frecuencia real del micrófono SPH0645
rate_vosk = 16000      # Frecuencia esperada por Vosk
recognizer = KaldiRecognizer(model, rate_vosk, '["entrance", "on", "down", "exit"]')
is_active = False

# --- Captura desde el micrófono correcto ---
cmd = [
    "arecord",
    "-D", "hw:1,0",      # Micrófono SPH0645
    "-f", "S32_LE",      # Formato nativo
    "-r", str(rate_hw),  # 32 kHz
    "-c", "1",           # Mono
    "-q",                # Silencioso
    "-t", "raw"          # Salida cruda
]
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, bufsize=0)

print("Escuchando micrófono (SPH0645)... di 'entrance' para activar control de voz")

# --- Función para normalizar el audio ---
def normalize_audio(data_32):
    data_f = data_32.astype(np.float32)
    rms = np.sqrt(np.mean(data_f ** 2))
    if rms < 1:
        rms = 1
    gain = 20000.0 / rms
    data_norm = np.clip(data_f * gain, -32768, 32767)
    return data_norm.astype(np.int16)

# --- Bucle principal ---
while True:
    data = proc.stdout.read(4000 * 4)
    if not data:
        continue

    # Conversión y re-muestreo
    data_32 = np.frombuffer(data, dtype=np.int32)
    data_16 = normalize_audio(data_32)
    data_16_resampled = data_16[::2]  # 32k ? 16k

    # Procesar con Vosk
    if recognizer.AcceptWaveform(data_16_resampled.tobytes()):
        text = json.loads(recognizer.Result()).get("text", "").lower()
        if not text:
            continue

        print(f"Reconocido: {text}")

        if not is_active:
            if "entrance" in text:
                is_active = True
                print("Voice control activado. Di 'on' para encender el LED o 'down' para apagarlo.")
        else:
            if "on" in text:
                print("LED ON")
                os.system("/home/root/led_on.sh")
            elif "down" in text:
                print("LED OFF")
                os.system("/home/root/led_off.sh")
            elif "exit" in text:
                print("Saliendo del asistente de voz.")
                proc.terminate()
                break


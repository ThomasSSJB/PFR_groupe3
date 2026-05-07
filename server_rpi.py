#!/usr/bin/env python3
"""
FICHIER: server_rpi.py
RÔLE: Serveur web Flask sur Raspberry Pi 3B

PRINCIPE :
  - /commande  → écrit dans data/commande.txt
                 main.c thread détecte → traiter_commande() → action.txt
  - /signal    → Flask appelle directement pilote_robot.py ou simulation.py
                 (comme taper 4 ou 3 dans le menu terminal)
  - /joystick  → envoie directement l'octet à l'Arduino
  - /audio     → STT → écrit dans data/commande.txt
  - /video     → flux Pi Camera MJPEG
"""

import os
import time
import threading
import subprocess
import tempfile
import serial
from flask import Flask, Response, request, jsonify
from flask_cors import CORS
import speech_recognition as sr

# ============================================================
# CONFIGURATION
# ============================================================

ARDUINO_PORT = "/dev/ttyACM0"
ARDUINO_BAUD = 9600

BASE_DIR    = os.path.dirname(os.path.abspath(__file__))
CMD_FILE    = os.path.join(BASE_DIR, "data/commande.txt")
ACTION_FILE = os.path.join(BASE_DIR, "data/action.txt")

WEB_PORT    = 5000

# ============================================================
# FLASK
# ============================================================

app = Flask(__name__)
CORS(app)

# ============================================================
# ARDUINO — joystick uniquement
# ============================================================

arduino = None

def connecter_arduino():
    global arduino
    try:
        arduino = serial.Serial(ARDUINO_PORT, ARDUINO_BAUD, timeout=1)
        time.sleep(2)
        print(f"[ARDUINO] Connecté sur {ARDUINO_PORT}")
    except Exception as e:
        print(f"[ARDUINO] Non connecté : {e}")
        arduino = None

def envoyer_arduino(octet: str):
    global arduino
    if arduino and arduino.is_open:
        try:
            arduino.write(octet.encode("utf-8"))
            print(f"[ARDUINO] >>> '{octet}'")
        except Exception as e:
            print(f"[ARDUINO] Erreur : {e}")
    else:
        print(f"[ARDUINO] (non connecté) octet='{octet}'")

# ============================================================
# CAMÉRA PI CAMERA via rpicam-vid
# ============================================================

camera_lock   = threading.Lock()
current_frame = None

def init_camera():
    global current_frame
    print("[CAMERA] Démarrage rpicam-vid...")
    cmd = [
        "rpicam-vid", "-t", "0", "--codec", "mjpeg",
        "--nopreview", "-o", "-"
    ]
    try:
        proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
        buffer = b""
        while True:
            chunk = proc.stdout.read(4096)
            if not chunk:
                break
            buffer += chunk
            start = buffer.find(b'\xff\xd8')
            end   = buffer.find(b'\xff\xd9')
            if start != -1 and end != -1:
                jpg = buffer[start:end+2]
                buffer = buffer[end+2:]
                with camera_lock:
                    current_frame = jpg
    except Exception as e:
        print(f"[CAMERA] rpicam-vid échoué ({e}), mode démo")
        _camera_demo()

def _camera_demo():
    global current_frame
    try:
        import cv2, numpy as np
        while True:
            img = np.zeros((480, 640, 3), dtype=np.uint8)
            img[:] = (20, 25, 30)
            cv2.putText(img, "PAS DE CAMERA", (170, 220),
                        cv2.FONT_HERSHEY_SIMPLEX, 1.2, (60, 200, 100), 2)
            cv2.putText(img, time.strftime("%H:%M:%S"), (255, 270),
                        cv2.FONT_HERSHEY_SIMPLEX, 1, (80, 80, 200), 2)
            _, buf = cv2.imencode('.jpg', img)
            with camera_lock:
                current_frame = buf.tobytes()
            time.sleep(0.1)
    except Exception as e:
        print(f"[CAMERA] Demo impossible : {e}")

def generate_mjpeg():
    while True:
        with camera_lock:
            frame = current_frame
        if frame:
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
        time.sleep(0.04)

# ============================================================
# ROUTES
# ============================================================

@app.route("/")
def index():
    return jsonify({"statut": "serveur robot actif"})

@app.route("/video")
def video_feed():
    return Response(generate_mjpeg(),
                    mimetype="multipart/x-mixed-replace; boundary=frame")

# ── Commande texte → commande.txt → main.c thread → traiter_commande()
@app.route("/commande", methods=["POST"])
def recevoir_commande():
    data = request.get_json()
    if not data or "texte" not in data:
        return jsonify({"erreur": "champ 'texte' manquant"}), 400

    texte = data["texte"].strip()
    print(f"[COMMANDE] '{texte}' → commande.txt")

    os.makedirs(os.path.dirname(CMD_FILE), exist_ok=True)
    with open(CMD_FILE, "w", encoding="utf-8") as f:
        f.write(texte + "\n")

    return jsonify({"statut": "ok"})

# ── Lancer robot / simulation → subprocess direct (comme option 4 / 3 du menu)
@app.route("/signal", methods=["POST"])
def recevoir_signal():
    data = request.get_json()
    if not data or "type" not in data:
        return jsonify({"erreur": "champ 'type' manquant"}), 400

    signal = data["type"]

    if signal == "lancer_robot":
        print("[WEB] Lancement robot (comme option 4 du menu)")
        subprocess.Popen(["python3", "python/pilote_robot.py"], cwd=BASE_DIR)
        return jsonify({"statut": "ok"})


    return jsonify({"erreur": "signal inconnu"}), 400


@app.route("/lidar_log")
def lidar_log():
    """Stream la sortie de display_map.py en temps reel."""
    def generer():
        cmd = ". ~/code/PFR_groupe3/venv/bin/activate && python3 python/scan_lidar.py | python3 python/display_map.py"
        proc = subprocess.Popen(
            cmd, shell=True, cwd=BASE_DIR,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT  # melange stdout + stderr
        )
        for line in iter(proc.stdout.readline, b""):
            yield f"data: {line.decode().rstrip()}\n\n"
        proc.wait()
        yield "data: [TERMINE]\n\n"

    return Response(generer(), mimetype="text/event-stream",
                    headers={"Cache-Control": "no-cache",
                             "X-Accel-Buffering": "no"})
# ── Joystick → Arduino direct
@app.route("/joystick", methods=["POST"])
def joystick():
    data = request.get_json()
    if not data or "octet" not in data:
        return jsonify({"erreur": "champ 'octet' manquant"}), 400

    octet = data["octet"]
    mapping = {
        "1": "avancer", "2": "reculer", "3": "stop",
        "4": "gauche",  "5": "droite",
        "6": "vitesse+","7": "vitesse-",
        "8": "klaxon",  "10": "aspirateur"
    }
    action = mapping.get(octet, f"cmd_{octet}")
    envoyer_arduino(octet)
    return jsonify({"octet": octet, "action": action, "statut": "ok"})

# ── Audio → STT → commande.txt → main.c thread → traiter_commande()
@app.route("/audio", methods=["POST"])
def recevoir_audio():
    if "audio" not in request.files:
        return jsonify({"erreur": "pas de fichier audio"}), 400

    raw = request.files["audio"].read()
    print(f"[AUDIO] Reçu {len(raw)/1024:.1f} Ko")

    tmp_webm = tempfile.NamedTemporaryFile(suffix=".webm", delete=False)
    tmp_wav  = tmp_webm.name.replace(".webm", ".wav")

    try:
        tmp_webm.write(raw)
        tmp_webm.close()

        res = subprocess.run(
            ["ffmpeg", "-y", "-i", tmp_webm.name,
             "-ar", "16000", "-ac", "1", "-f", "wav", tmp_wav],
            capture_output=True, timeout=10
        )
        if res.returncode != 0:
            return jsonify({"erreur": "conversion audio echouee"}), 500

        r = sr.Recognizer()
        with sr.AudioFile(tmp_wav) as source:
            audio = r.record(source)
        texte = r.recognize_google(audio, language="fr-FR")
        print(f"[STT] '{texte}' → commande.txt")

    except sr.UnknownValueError:
        return jsonify({"texte": "", "statut": "incompris"})
    except sr.RequestError as e:
        return jsonify({"erreur": f"STT API: {e}"}), 503
    except subprocess.TimeoutExpired:
        return jsonify({"erreur": "ffmpeg timeout"}), 500
    finally:
        for fp in [tmp_webm.name, tmp_wav]:
            try: os.unlink(fp)
            except: pass

    with open(CMD_FILE, "w", encoding="utf-8") as f:
        f.write(texte + "\n")

    return jsonify({"texte": texte, "statut": "ok"})

@app.route("/status")
def status():
    derniere = ""
    try:
        if os.path.exists(ACTION_FILE):
            with open(ACTION_FILE) as f:
                lignes = [l.strip() for l in f if l.strip()]
                derniere = lignes[-1] if lignes else ""
    except: pass
    return jsonify({
        "arduino_connecte": arduino is not None and arduino.is_open,
        "derniere_action":  derniere,
        "heure":            time.strftime("%H:%M:%S")
    })

@app.route("/reset", methods=["POST"])
def reset():
    open(ACTION_FILE, "w").close()
    open(ACTION_FILE, "w").close()
    envoyer_arduino("3")
    return jsonify({"statut": "reset ok"})

@app.route("/carte")
def carte():
    """Sert la carte LiDAR sauvegardée dans data/lidar_map.png"""
    from flask import send_file
    carte_path = os.path.join(BASE_DIR, "data/lidar_map.png")
    
    cmd = (
        ". ~/code/PFR_groupe3/venv/bin/activate && "
        "python3 python/scan_lidar.py | "
        "python3 python/display_map.py"
    )

    subprocess.run(cmd, shell=True, cwd=BASE_DIR)
    
    if not os.path.exists(carte_path):
        return jsonify({"carte non disponible"}), 404
    return send_file(carte_path, mimetype="image/png")

# ============================================================
# MAIN
# ============================================================

if __name__ == "__main__":
    os.makedirs(os.path.join(BASE_DIR, "data"), exist_ok=True)
    connecter_arduino()
    threading.Thread(target=init_camera, daemon=True).start()
    time.sleep(2)
    print(f"\n[SERVEUR] http://0.0.0.0:{WEB_PORT}")
    print(f"[SERVEUR] IMPORTANT : lancer 'make run' en parallele\n")
    app.run(host="0.0.0.0", port=WEB_PORT, debug=False, threaded=True)

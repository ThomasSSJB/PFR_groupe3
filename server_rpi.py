
#!/usr/bin/env python3
"""
FICHIER: server_rpi.py
RÔLE: Serveur web Flask sur Raspberry Pi 3B
      - Streaming Pi Camera (MJPEG)
      - Réception audio depuis navigateur → conversion ffmpeg → Google STT
      - Réception commandes texte/joystick → commande.txt + action.txt
      - Envoi commandes vers Arduino via USB Serial (/dev/ttyACM0)

PORTS:
  - Arduino : /dev/ttyACM0 (câble USB) → lu sur Serial dans robot_corrige_v31.ino
  - LiDAR   : /dev/ttyUSB0 (si présent, géré séparément par scan_lidar.py)

OCTETS ARDUINO (robot_corrige_v31.ino):
  '1'  → avancer
  '2'  → reculer
  '3'  → stop
  '4'  → gauche
  '5'  → droite
  '6'  → vitesse +20
  '7'  → vitesse -20
  '8'  → klaxon
  '10' → mode aspirateur ON/OFF
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
# ARDUINO — USB Serial (/dev/ttyACM0)
# ============================================================

arduino = None

def connecter_arduino():
    global arduino
    try:
        arduino = serial.Serial(ARDUINO_PORT, ARDUINO_BAUD, timeout=1)
        time.sleep(2)
        print(f"[ARDUINO] Connecté sur {ARDUINO_PORT} @ {ARDUINO_BAUD} baud")
    except Exception as e:
        print(f"[ARDUINO] Non connecté : {e}")
        arduino = None

def envoyer_arduino(octet: str):
    """
    Envoie un octet à l'Arduino Mega via USB.
    robot_corrige_v31.ino lit sur Serial (USB) ET Serial1 (Bluetooth).
    """
    global arduino
    if arduino and arduino.is_open:
        try:
            arduino.write(octet.encode("utf-8"))
            print(f"[ARDUINO] >>> '{octet}'")
        except Exception as e:
            print(f"[ARDUINO] Erreur envoi : {e}")
    else:
        print(f"[ARDUINO] (non connecté) octet='{octet}'")

# ============================================================
# CAMÉRA PI CAMERA (RPi 3B)
# ============================================================

camera_lock   = threading.Lock()
current_frame = None

def init_camera():
    global current_frame
    import subprocess

    print("[CAMERA] dmarrage rpicam...")

    cmd = [
        "rpicam-vid",
        "-t", "0",
        "--codec", "mjpeg",
        "--nopreview", 
        "--width", "640",
        "--height", "480",
        "--framerate", "30",
        "-o", "-"
    ]

    proc = subprocess.Popen(cmd, stdout=subprocess.PIPE)

    buffer = b""

    while True:
        chunk = proc.stdout.read(4096)
        if not chunk:
            continue

        buffer += chunk

        start = buffer.find(b'\xff\xd8')  # JPEG start
        end = buffer.find(b'\xff\xd9')    # JPEG end

        if start != -1 and end != -1:
            jpg = buffer[start:end+2]
            buffer = buffer[end+2:]

            with camera_lock:
                current_frame = jpg

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
# PARSEUR — miroir Python de commande_vocale.c
# Retourne (action_txt, octet_arduino)
# ============================================================

MOTS_AVANCE = ["avance","avancer","advance","forward","va","vas"]
MOTS_RECUL  = ["recule","reculer","retreat","backward","arriere","arrière"]
MOTS_TOURNE = ["tourne","tourner","turn","pivote"]
MOTS_GAUCHE = ["gauche","left"]
MOTS_DROITE = ["droite","right"]
MOTS_STOP   = ["stop","arrête","arrete","halt","pause"]
MOTS_BALLE  = ["balle","ball","ballon"]
MOTS_TROUVE = ["trouve","trouver","cherche","find","attrape"]
MOTS_KLAXON = ["klaxon","klaxonne","beep","bip"]
MOTS_ASPIR  = ["aspirateur","auto","autonome","explore"]
COULEURS    = {
    "rouge":"red","red":"red",
    "bleu":"blue","blue":"blue",
    "vert":"green","green":"green",
    "jaune":"yellow","yellow":"yellow"
}

def parser_commande(texte: str):
    mots = texte.lower().strip().split()

    if any(m in MOTS_STOP for m in mots):
        return "stop", "3"

    if any(m in MOTS_KLAXON for m in mots):
        return "klaxon", "8"

    if any(m in MOTS_ASPIR for m in mots):
        return "aspirateur", "10"

    trouve  = any(m in MOTS_TROUVE for m in mots)
    balle   = any(m in MOTS_BALLE for m in mots)
    couleur = next((COULEURS[m] for m in mots if m in COULEURS), None)
    if trouve and balle and couleur:
        return f"find_ball {couleur}", None  # géré par pilote_robot.py

    if any(m in MOTS_TOURNE for m in mots):
        direction = "left" if any(m in MOTS_GAUCHE for m in mots) else "right"
        angle = next((int(m) for m in mots if m.isdigit()), 90)
        return f"turn {direction} {angle}", "4" if direction == "left" else "5"

    if any(m in MOTS_AVANCE for m in mots):
        dist = next((int(m) for m in mots if m.isdigit()), 50)
        return f"advance {dist}", "1"

    if any(m in MOTS_RECUL for m in mots):
        dist = next((int(m) for m in mots if m.isdigit()), 50)
        return f"retreat {dist}", "2"

    return "", None

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

@app.route("/commande", methods=["POST"])
def recevoir_commande():
    """
    Commande texte depuis l'interface web.
    Écrit dans commande.txt + action.txt, envoie à l'Arduino.
    """
    data = request.get_json()
    if not data or "texte" not in data:
        return jsonify({"erreur": "champ 'texte' manquant"}), 400

    texte = data["texte"].strip()
    print(f"[COMMANDE] '{texte}'")

    os.makedirs(os.path.dirname(CMD_FILE), exist_ok=True)
    with open(CMD_FILE, "w", encoding="utf-8") as f:
        f.write(texte + "\n")

    action, octet = parser_commande(texte)
    if action:
        with open(ACTION_FILE, "a", encoding="utf-8") as f:
            f.write(action + "\n")
        
        return jsonify({"action": action, "octet": octet, "statut": "ok"})

    return jsonify({"action": None, "statut": "commande_inconnue"})

@app.route("/joystick", methods=["POST"])
def joystick():
    """
    Boutons joystick → envoie directement l'octet à l'Arduino.
    Plus rapide que /commande pour le contrôle temps réel.
    JSON: { "octet": "1" }
    """
    data = request.get_json()
    if not data or "octet" not in data:
        return jsonify({"erreur": "champ 'octet' manquant"}), 400

    octet = data["octet"]
    mapping = {
        "1":"advance 50","2":"retreat 50","3":"stop",
        "4":"turn left 45","5":"turn right 45",
        "6":"vitesse+","7":"vitesse-","8":"klaxon","10":"aspirateur"
    }
    action = mapping.get(octet, f"cmd_{octet}")
    envoyer_arduino(octet)

    with open(ACTION_FILE, "a", encoding="utf-8") as f:
        f.write(action + "\n")

    return jsonify({"octet": octet, "action": action, "statut": "ok"})

@app.route("/audio", methods=["POST"])
def recevoir_audio():
    """
    Reçoit audio WebM depuis le navigateur.
    Convertit en WAV via ffmpeg → Google STT → même pipeline que /commande.
    
    Prérequis RPi: sudo apt install ffmpeg
    """
    if "audio" not in request.files:
        return jsonify({"erreur": "pas de fichier audio"}), 400

    raw = request.files["audio"].read()
    print(f"[AUDIO] Reçu {len(raw)/1024:.1f} Ko")

    tmp_webm = tempfile.NamedTemporaryFile(suffix=".webm", delete=False)
    tmp_wav  = tmp_webm.name.replace(".webm", ".wav")

    try:
        tmp_webm.write(raw)
        tmp_webm.close()

        # Conversion WebM → WAV 16kHz mono
        res = subprocess.run(
            ["ffmpeg", "-y", "-i", tmp_webm.name,
             "-ar", "16000", "-ac", "1", "-f", "wav", tmp_wav],
            capture_output=True, timeout=10
        )
        if res.returncode != 0:
            print(f"[AUDIO] ffmpeg erreur : {res.stderr.decode()}")
            return jsonify({"erreur": "conversion audio échouée"}), 500

        # Google STT
        r = sr.Recognizer()
        with sr.AudioFile(tmp_wav) as source:
            audio = r.record(source)
        texte = r.recognize_google(audio, language="fr-FR")
        print(f"[STT] '{texte}'")

    except sr.UnknownValueError:
        return jsonify({"texte":"","action":None,"statut":"incompris"})
    except sr.RequestError as e:
        return jsonify({"erreur": f"STT API: {e}"}), 503
    except subprocess.TimeoutExpired:
        return jsonify({"erreur": "ffmpeg timeout"}), 500
    finally:
        for f in [tmp_webm.name, tmp_wav]:
            try: os.unlink(f)
            except: pass

    # Écriture et envoi
    with open(CMD_FILE, "w", encoding="utf-8") as f:
        f.write(texte + "\n")

    action, octet = parser_commande(texte)
    if action:
        with open(ACTION_FILE, "a", encoding="utf-8") as f:
            f.write(action + "\n")
        if octet:
            envoyer_arduino(octet)

    return jsonify({"texte": texte, "action": action, "octet": octet, "statut": "ok"})

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
        "arduino_port":     ARDUINO_PORT,
        "derniere_action":  derniere,
        "heure":            time.strftime("%H:%M:%S")
    })

@app.route("/reset", methods=["POST"])
def reset():
    open(ACTION_FILE, "w").close()
    envoyer_arduino("3")
    return jsonify({"statut": "reset ok"})

# ============================================================
# MAIN
# ============================================================

if __name__ == "__main__":
    os.makedirs(os.path.join(BASE_DIR, "data"), exist_ok=True)

    connecter_arduino()

    threading.Thread(target=init_camera, daemon=True).start()
    time.sleep(2)

    print(f"\n[SERVEUR] Démarrage sur http://0.0.0.0:{WEB_PORT}")
    print(f"[SERVEUR] Flux caméra : http://<IP-RPi>:{WEB_PORT}/video\n")

    app.run(host="0.0.0.0", port=WEB_PORT, debug=False, threaded=True)

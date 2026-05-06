#!/usr/bin/env python3
"""
FICHIER: server_rpi.py
AUTEUR: Hajar
RÔLE: Serveur Flask qui fait le lien entre l'interface web et le robot

FLUX:
  Boutons/Voix/Texte (interface)
       ↓
  Flask (ce fichier)
       ↓ écrit dans
  commande.txt → traiter_commande (C) → action.txt
  OU action.txt directement (joystick)
       ↓
  pilote_robot.py → Arduino → Robot
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

ARDUINO_PORT  = "/dev/ttyACM0"
ARDUINO_BAUD  = 9600

BASE_DIR      = os.path.dirname(os.path.abspath(__file__))
# Chemins relatifs au dossier python/
CMD_FILE      = os.path.join(BASE_DIR, "data/commande.txt")
ACTION_FILE   = os.path.join(BASE_DIR, "..", "data/action.txt")
PILOTE_SCRIPT = os.path.join(BASE_DIR, "pilote_robot.py")

WEB_PORT      = 5000

# ============================================================
# FLASK
# ============================================================

app = Flask(__name__)
CORS(app)  # autorise les requêtes depuis le navigateur

# ============================================================
# ARDUINO
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

def envoyer_octet(octet: str):
    """Envoie un octet directement à l'Arduino (pour le joystick temps réel)."""
    global arduino
    if arduino and arduino.is_open:
        try:
            arduino.write(octet.encode("utf-8"))
            print(f"[ARDUINO] Octet envoyé : '{octet}'")
        except Exception as e:
            print(f"[ARDUINO] Erreur envoi : {e}")
    else:
        print(f"[ARDUINO] Non connecté, octet ignoré : '{octet}'")

# ============================================================
# CAMÉRA via rpicam-vid (Pi Camera)
# ============================================================

camera_lock   = threading.Lock()
current_frame = None

def init_camera():
    global current_frame
    print("[CAMERA] Démarrage rpicam-vid...")
    cmd = [
        "rpicam-vid", "-t", "0", "--codec", "mjpeg",
        "--nopreview", "--width", "640", "--height", "480",
        "--framerate", "20", "-o", "-"
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
        print(f"[CAMERA] Erreur : {e}")

def generate_mjpeg():
    while True:
        with camera_lock:
            frame = current_frame
        if frame:
            yield (b'--frame\r\nContent-Type: image/jpeg\r\n\r\n' + frame + b'\r\n')
        time.sleep(0.05)

# ============================================================
# ROUTES FLASK
# ============================================================

@app.route("/")
def index():
    return jsonify({"statut": "serveur robot actif"})

# --- Caméra ---
@app.route("/video")
def video():
    return Response(generate_mjpeg(),
                    mimetype="multipart/x-mixed-replace; boundary=frame")

# --- Status ---
@app.route("/status")
def status():
    return jsonify({
        "arduino": arduino is not None and arduino.is_open,
        "heure":   time.strftime("%H:%M:%S")
    })

# --- Joystick temps réel → octet direct à Arduino ---
@app.route("/joystick", methods=["POST"])
def joystick():
    data = request.get_json()
    if not data or "octet" not in data:
        return jsonify({"erreur": "octet manquant"}), 400

    octet = data["octet"]
    labels = {
        "1": "avancer", "2": "reculer", "3": "stop",
        "4": "gauche",  "5": "droite"
    }
    envoyer_octet(octet)
    return jsonify({"statut": "ok", "action": labels.get(octet, octet)})

# ============================================================
# INTERPRÉTEUR DE COMMANDES NATURELLES
# ============================================================

# Mots-clés reconnus (basés sur fr.json + en.json)
MOTS_AVANCE  = ["avance", "avant", "marche", "avancer", "aller", "forward", "advance"]
MOTS_RECULE  = ["recule", "arrière", "reculer", "recul", "retreat", "backward", "back"]
MOTS_GAUCHE  = ["gauche", "left"]
MOTS_DROITE  = ["droite", "right"]
MOTS_STOP    = ["stop", "arrêt", "arrêter", "halt"]
MOTS_TOURNE  = ["tourne", "tourner", "pivoter", "rotation", "turn", "rotate"]
MOTS_SEPARATEURS = ["et", "puis", "ensuite", "after", "then"]

def extraire_nombre(mots, index):
    """Cherche un nombre dans les mots suivants."""
    for i in range(index, min(index + 4, len(mots))):
        try:
            return int(mots[i])
        except ValueError:
            pass
    return None

def interpreter_commande(texte):
    """
    Transforme une phrase naturelle en liste d'actions.
    Exemple : "avance de 2 mètres et recule de 1 mètre puis tourne à gauche"
    → ["advance 2", "retreat 1", "turn left 90"]
    """
    # Nettoyer le texte
    texte = texte.lower()
    # Supprimer les mots parasites
    for mot in ["de", "du", "la", "le", "les", "à", "au", "mètre", "mètres",
                "m", "degré", "degrés", "°", "metre", "metres"]:
        texte = texte.replace(mot, " ")

    mots = texte.split()
    actions = []
    i = 0

    while i < len(mots):
        mot = mots[i]

        # AVANCER
        if mot in MOTS_AVANCE:
            distance = extraire_nombre(mots, i + 1) or 50
            actions.append(f"advance {distance}")
            print(f"[INTERP] advance {distance}")
            i += 1
            continue

        # RECULER
        if mot in MOTS_RECULE:
            distance = extraire_nombre(mots, i + 1) or 50
            actions.append(f"retreat {distance}")
            print(f"[INTERP] retreat {distance}")
            i += 1
            continue

        # TOURNER
        if mot in MOTS_TOURNE:
            angle = extraire_nombre(mots, i + 1) or 90
            direction = "left"  # par défaut
            # Cherche gauche ou droite dans les mots suivants
            for j in range(i + 1, min(i + 5, len(mots))):
                if mots[j] in MOTS_GAUCHE:
                    direction = "left"
                    break
                if mots[j] in MOTS_DROITE:
                    direction = "right"
                    break
            actions.append(f"turn {direction} {angle}")
            print(f"[INTERP] turn {direction} {angle}")
            i += 1
            continue

        # STOP
        if mot in MOTS_STOP:
            actions.append("stop")
            print(f"[INTERP] stop")
            i += 1
            continue

        i += 1

    return actions

# --- Commande texte → interpréteur → action.txt → pilote ---
@app.route("/commande_texte", methods=["POST"])
def commande_texte():
    data = request.get_json()
    if not data or "texte" not in data:
        return jsonify({"erreur": "texte manquant"}), 400

    texte = data["texte"].strip()
    print(f"[TEXTE] Reçu : '{texte}'")

    # 1. Écrire dans commande.txt
    cmd_file = os.path.join(BASE_DIR, "..", "data/commande.txt")
    with open(cmd_file, "w", encoding="utf-8") as f:
        f.write(texte + "\n")

    # 2. Lancer make run et simuler choix 1 (écrire commande) puis 5 (quitter)
    try:
        subprocess.run(
            ["./bin/prog_principal.out"],
            cwd=os.path.join(BASE_DIR, ".."),
            input="5402\n1\n\n5\n",
            capture_output=True, text=True, timeout=15
        )
        print(f"[TEXTE] traiter_commande() exécuté ✓")
    except Exception as e:
        print(f"[TEXTE] Erreur : {e}")

    return jsonify({"statut": "ok", "commande": texte})

# --- Commande vocale → Google STT → commande.txt → pilote ---
@app.route("/audio", methods=["POST"])
def audio():
    if "audio" not in request.files:
        return jsonify({"erreur": "pas d'audio"}), 400

    raw = request.files["audio"].read()
    print(f"[AUDIO] Reçu {len(raw)/1024:.1f} Ko")

    tmp_webm = tempfile.NamedTemporaryFile(suffix=".webm", delete=False)
    tmp_wav  = tmp_webm.name.replace(".webm", ".wav")

    try:
        tmp_webm.write(raw)
        tmp_webm.close()

        # Conversion WebM → WAV
        subprocess.run(
            ["ffmpeg", "-y", "-i", tmp_webm.name,
             "-ar", "16000", "-ac", "1", "-f", "wav", tmp_wav],
            capture_output=True, timeout=10
        )

        # Google STT
        r = sr.Recognizer()
        with sr.AudioFile(tmp_wav) as source:
            audio_data = r.record(source)
        texte = r.recognize_google(audio_data, language="fr-FR")
        print(f"[STT] Reconnu : '{texte}'")

    except sr.UnknownValueError:
        return jsonify({"texte": "", "statut": "incompris"})
    except Exception as e:
        return jsonify({"erreur": str(e)}), 500
    finally:
        for fp in [tmp_webm.name, tmp_wav]:
            try: os.unlink(fp)
            except: pass

    # Interpréter et écrire dans action.txt
    actions = interpreter_commande(texte)
    if actions:
        with open(ACTION_FILE, "w", encoding="utf-8") as f:
            for action in actions:
                f.write(action + "\n")
        print(f"[AUDIO] {len(actions)} action(s) écrites")

    return jsonify({"texte": texte, "statut": "ok", "actions": actions})

# --- Lancer le robot → exécute action.txt via pilote_robot.py ---
@app.route("/lancer", methods=["POST"])
def lancer():
    print("[LANCER] Démarrage pilote_robot.py")
    subprocess.Popen(["python3", PILOTE_SCRIPT])
    return jsonify({"statut": "ok"})

# --- Reset ---
@app.route("/reset", methods=["POST"])
def reset():
    open(ACTION_FILE, "w").close()
    envoyer_octet("3")
    return jsonify({"statut": "reset ok"})

# ============================================================
# DÉMARRAGE
# ============================================================

if __name__ == "__main__":
    os.makedirs(os.path.join(BASE_DIR, "data"), exist_ok=True)

    connecter_arduino()
    threading.Thread(target=init_camera, daemon=True).start()
    time.sleep(1)

    print(f"\n[SERVEUR] Démarrage sur http://0.0.0.0:{WEB_PORT}")
    print(f"[SERVEUR] Interface : ouvre interface_h.html dans ton navigateur\n")

    app.run(host="0.0.0.0", port=5000, debug=False, threaded=True)

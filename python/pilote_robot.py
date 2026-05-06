import serial
import time
import os
import cv2
import numpy as np
import math as m

# Configuration des chemins
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
FICHIER_ACTION = os.path.join(BASE_DIR, "..", "data/action.txt")

# Connexion à l'Arduino
try:
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    time.sleep(2)  # Temps pour que l'Arduino reboot
    print("[ROBOT] Connexion Arduino OK")
except:
    print("[ROBOT] Erreur : Arduino non trouvée sur /dev/ttyACM0")
    exit()

def lire_actions():
    if not os.path.exists(FICHIER_ACTION):
        return []
    with open(FICHIER_ACTION, "r", encoding="utf-8") as f:
        return [l.strip() for l in f if l.strip()]

def envoyer_arduino(commande, duree=1):
    print(f"[ROBOT] >>> ENVOI OCTET: '{commande}' pendant {duree}s")
    ser.write(commande.encode())
    time.sleep(duree)
    ser.write(b'3')
    time.sleep(0.1)


def detecter_balle(couleur):
    """
    Détecte une balle de couleur spécifiée et retourne sa position.
    Filtre par aire ET circularité pour éviter les faux positifs.
    Retourne: (cx, cy, radius) ou None si aucune balle n'est trouvée
    """
    color_ranges = {
        "red": [
            (np.array([0, 120, 70]),   np.array([10, 255, 255])),
            (np.array([170, 120, 70]), np.array([180, 255, 255]))
        ],
        "blue":   [(np.array([100, 120, 70]), np.array([130, 255, 255]))],
        "green":  [(np.array([50, 120, 70]),  np.array([90, 255, 255]))],
        "yellow": [(np.array([20, 120, 70]),  np.array([50, 255, 255]))]
    }

    chemin_brute    = os.path.join(BASE_DIR, "..", "data/photo_brute.jpg")
    chemin_detectee = os.path.join(BASE_DIR, "..", "data/photo_detectee.jpg")

    # 1640x1232 = plein capteur (pas de crop), resize ensuite en RAM
    ret = os.system(f"rpicam-jpeg --nopreview --immediate --width 3280 --height 2464 -o {chemin_brute}")

    if ret != 1:
        frame = cv2.imread(chemin_brute)
        if frame is None:
            print("[ROBOT] ❌ Erreur lecture image")
            return None

        # Redimensionner en 640x480 (plein FOV conservé, traitement rapide)
        # frame = cv2.resize(frame, (3280, 2464))
        print(f"[ROBOT] 📷 Photo brute sauvegardée: {chemin_brute}")

        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

        mask = None
        if couleur.lower() in color_ranges:
            for lower, upper in color_ranges[couleur.lower()]:
                if mask is None:
                    mask = cv2.inRange(hsv, lower, upper)
                else:
                    mask += cv2.inRange(hsv, lower, upper)
        else:
            print(f"[ROBOT] Couleur '{couleur}' non reconnue")
            return None

        mask = cv2.medianBlur(mask, 5)
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

        if contours:
            meilleur       = None
            meilleure_aire = 0

            for c in contours:
                aire      = cv2.contourArea(c)
                perimetre = cv2.arcLength(c, True)

                # Ignorer les contours trop petits
                if aire < 500 or perimetre == 0:
                    continue

                # Circularité : 1.0 = cercle parfait
                # Formule : 4π × aire / périmètre²
                circularite = (4 * m.pi * aire) / (perimetre ** 2)

                print(f"[ROBOT] Contour : aire={aire:.0f}, circularité={circularite:.2f}")

                # Garder uniquement les formes circulaires (>= 0.7) et prendre la plus grande
                if circularite >= 0.7 and aire > meilleure_aire:
                    meilleure_aire = aire
                    meilleur       = c

            if meilleur is not None:
                (cx, cy), radius = cv2.minEnclosingCircle(meilleur)
                cx, cy, radius   = int(cx), int(cy), int(radius)

                print(f"[ROBOT] ✓ Balle {couleur} détectée : ({cx}, {cy}), rayon: {radius}")

                cv2.circle(frame, (cx, cy), radius, (0, 255, 0), 2)
                cv2.circle(frame, (cx, cy), 5, (0, 0, 255), -1)
                cv2.imwrite(chemin_detectee, frame)
                print(f"[ROBOT] 📸 Photo avec détection sauvegardée: {chemin_detectee}")

                return (cx, cy, radius)
            else:
                print(f"[ROBOT] ⚠ Aucune forme circulaire {couleur} détectée (circularité < 0.7 ou trop petit)")
        else:
            print(f"[ROBOT] ⚠ Aucune balle {couleur} détectée")
    else:
        print("[ROBOT] ❌ Erreur capture caméra")

    return None


def envoyer_position_balle(cx, cy, radius, frame_width=3280, frame_height=2464):
    """
    Envoie des commandes au robot en fonction de la position de la balle.
    """
    angle   = m.atan(abs(frame_width // 2 - cx) / abs(frame_height - cy))
    angle   = angle * 180 / m.pi * 1.05
    duree_t = 0.65 / 90 * angle

    distance = 87 / radius - 0.15
    duree_a  = ((distance - 0.65) / 0.77) + 1

    print(f"[IMAGE] cx={cx}, cy={cy}, angle={angle:.2f}°, duree_t={duree_t:.3f}s")
    print(f"[IMAGE] radius={radius}, distance={distance:.2f}m, duree_a={duree_a:.3f}s")

    envoyer_arduino('6', duree=0.1)
    if cx > (frame_width // 2):
        print("[ROBOT] Tourne à droite")
        envoyer_arduino('5', duree_t)
    else:
        print("[ROBOT] Tourne à gauche")
        envoyer_arduino('4', duree_t)
    envoyer_arduino('7', duree=0.1)
    envoyer_arduino('1', duree_a)


def main():
    print("[ROBOT] Lecture des actions en cours...")
    actions = lire_actions()

    if not actions:
        print("[ROBOT] Aucune action à exécuter.")
        return

    for ligne in actions:
        parts  = ligne.split()
        action = parts[0]

        if action == "advance":
            distance = parts[1]
            duree = ((int(distance) - 0.65) / 0.77) + 1
            envoyer_arduino('1', duree)
        elif action == "retreat":
            distance = parts[1]
            duree = ((int(distance) - 0.65) / 0.77) + 1
            envoyer_arduino('2', duree)
        elif action == "turn":
            angle = parts[2]
            duree = 1.15 / 90 * int(angle)
            envoyer_arduino('6', duree=0.1)
            if parts[1] == "left":
                envoyer_arduino('4', duree)
            else:
                envoyer_arduino('5', duree)
            envoyer_arduino('7', duree=0.1)
        elif action == "find_ball":
            couleur = parts[1] if len(parts) > 1 else "red"
            print(f"[ROBOT] Recherche balle {couleur}...")

            iteration = 0
            resultat = detecter_balle(couleur)
            condition = True
            while condition and iteration < 12:
                if resultat:
                    condition = False
                    cx, cy, radius = resultat
                    envoyer_position_balle(cx, cy, radius)
                else:
                    print(f"[ROBOT] Impossible de détecter la balle {couleur}.")
                    iteration += 1
                    envoyer_arduino('6', duree=0.1)
                    envoyer_arduino('5', duree=0.5)
                    envoyer_arduino('7', duree=0.1)
                    resultat = detecter_balle(couleur)

        elif action == "stop":
            envoyer_arduino('3', duree=0.1)
            break

    open(FICHIER_ACTION, "w").close()
    print("[ROBOT] Toutes les actions ont été effectuées.")

if __name__ == "__main__":
    main()

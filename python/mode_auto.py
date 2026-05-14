#!/usr/bin/env python3
"""
mode_auto.py
Navigation LiDAR SIMPLE :
- avance si devant libre
- tourne vers le côté le plus libre sinon

Commandes :
1 = avance
3 = stop
4 = gauche
5 = droite
"""

import serial
import time
from rplidar import RPLidar

# ─────────────────────────────
# CONFIG
# ─────────────────────────────

LIDAR_PORT   = "/dev/ttyUSB0"
ARDUINO_PORT = "/dev/ttyACM0"

DIST_OBSTACLE = 300  # mm

# ─────────────────────────────
# CONNEXION
# ─────────────────────────────

print("Connexion Arduino...")
ser = serial.Serial(ARDUINO_PORT, 9600, timeout=1)

time.sleep(2)

print("Connexion LiDAR...")
lidar = RPLidar(LIDAR_PORT)

lidar.start_motor()
time.sleep(2)

lidar.clean_input()

# ─────────────────────────────
# ENVOI COMMANDE
# ─────────────────────────────

def envoyer(cmd):
    ser.write(f"{cmd}\n".encode())

# ─────────────────────────────
# DISTANCE SECTEUR
# ─────────────────────────────

def distance_secteur(data, centre, demi_angle=20):

    valeurs = []

    for a in range(
        centre - demi_angle,
        centre + demi_angle + 1
    ):

        d = data.get(a % 360)

        if d is not None and 80 < d < 5000:
            valeurs.append(d)

    if not valeurs:
        return 9999

    return min(valeurs)

# ─────────────────────────────
# BOUCLE PRINCIPALE
# ─────────────────────────────

print("🤖 Mode auto démarré")

try:

    for scan in lidar.iter_scans():

        data = {}

        for (_, angle, distance) in scan:

            if 80 < distance < 5000:
                data[int(angle) % 360] = distance

        # secteurs
        avant  = distance_secteur(data, 0)
        gauche = distance_secteur(data, 90)
        droite = distance_secteur(data, 270)

        print(
            f"Avant:{avant:.0f} "
            f"G:{gauche:.0f} "
            f"D:{droite:.0f}"
        )

        # ─────────────────────
        # obstacle devant
        # ─────────────────────

        if avant < DIST_OBSTACLE:

            envoyer(3)

            time.sleep(0.05)

            if gauche > droite:

                print("⬅️ Tourne gauche")

                envoyer(4)

            else:

                print("➡️ Tourne droite")

                envoyer(5)

            time.sleep(0.25)

            envoyer(3)

        # ─────────────────────
        # avance
        # ─────────────────────

        else:

            print("▶️ Avance")

            envoyer(1)

            time.sleep(0.1)

        time.sleep(0.03)

except KeyboardInterrupt:

    print("\n⛔ Arrêt")

finally:

    try:

        envoyer(3)

        lidar.stop()

        lidar.stop_motor()

        lidar.disconnect()

        ser.close()

    except:
        pass

    print("✅ Robot arrêté")
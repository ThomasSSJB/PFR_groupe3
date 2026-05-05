import sys
import serial
import time

# Connexion à l'Arduino
try:
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    time.sleep(2)
except:
    print("Erreur : Arduino non trouvée", file=sys.stderr)

def piloter_robot(commande):
    """ Envoie la commande à l'Arduino """
    ser.write(commande.encode())

# Lecture du flux LIDAR ligne par ligne
for line in sys.stdin:
    # On renvoie la ligne immédiatement pour que le PC la reçoive et dessine la carte
    sys.stdout.write(line)
    sys.stdout.flush()

    try:
        # On suppose le format : angle,distance
        parts = line.strip().split(",")
        angle = float(parts[0])
        distance = float(parts[1])

        # --- SCÉNARIO : BOUCLIER DE SÉCURITÉ ---
        # Si un objet est devant (340° à 20°) à moins de 500mm
        if (angle > 340 or angle < 20) and (100 < distance < 500):
            print(f"!!! OBSTACLE à {distance}mm !!!", file=sys.stderr)
            piloter_robot('3') # STOP
            time.sleep(0.1)
            piloter_robot('4') # Tourne à gauche pour chercher une sortie
        else:
            # Si c'est libre, on peut avancer
            # Note : on n'envoie '1' que si on n'est pas déjà en train de tourner
            piloter_robot('1')

    except:
        continue

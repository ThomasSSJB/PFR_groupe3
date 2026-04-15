import serial
import time
import os
import cv2
import numpy as np

# Configuration des chemins
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
FICHIER_ACTION = os.path.join(BASE_DIR, "..", "data/action.txt")

# Connexion à l'Arduino
try:
    ser = serial.Serial('/dev/ttyACM0', 9600, timeout=1)
    time.sleep(2) # Temps pour que l'Arduino reboot
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
    time.sleep(duree) # Le robot bouge pendant X secondes
    ser.write(b'3')   # On l'arrête après
    time.sleep(0.1)


def detecter_balle(couleur):
    """
    Détecte une balle de couleur spécifiée et retourne sa position.
    Retourne: (cx, cy, radius) ou None si aucune balle n'est trouvée
    """
    # Définition des plages HSV pour chaque couleur
    color_ranges = {
        "red": [
            (np.array([0, 120, 70]), np.array([10, 255, 255])),
            (np.array([170, 120, 70]), np.array([180, 255, 255]))
        ],
        "blue": [
            (np.array([100, 120, 70]), np.array([130, 255, 255]))
        ],
        "green": [
            (np.array([50, 120, 70]), np.array([90, 255, 255]))
        ],
        "yellow": [
            (np.array([20, 120, 70]), np.array([50, 255, 255]))
        ]
    }
    
    # Chemins de sauvegarde
    chemin_brute = os.path.join(BASE_DIR, "..", "data/photo_brute.jpg")
    chemin_detectee = os.path.join(BASE_DIR, "..", "data/photo_detectee.jpg")
    
    # Ouvrir caméra
    cap = cv2.VideoCapture(0)
    time.sleep(0.3)  # laisser la caméra s'initialiser
    
    ret, frame = cap.read()
    
    if ret:
        # Sauvegarde image brute
        cv2.imwrite(chemin_brute, frame)
        print(f"[ROBOT] 📷 Photo brute sauvegardée: {chemin_brute}")
        
        # Conversion HSV
        hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)
        
        # Créer le masque selon la couleur
        mask = None
        if couleur.lower() in color_ranges:
            for lower, upper in color_ranges[couleur.lower()]:
                if mask is None:
                    mask = cv2.inRange(hsv, lower, upper)
                else:
                    mask += cv2.inRange(hsv, lower, upper)
        else:
            print(f"[ROBOT] Couleur '{couleur}' non reconnue")
            cap.release()
            return None
        
        # Nettoyage
        mask = cv2.medianBlur(mask, 5)
        
        # Contours
        contours, _ = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)
        
        if contours:
            # Plus grosse balle
            c = max(contours, key=cv2.contourArea)
            
            if cv2.contourArea(c) > 500:
                (cx, cy), radius = cv2.minEnclosingCircle(c)
                
                cx = int(cx)
                cy = int(cy)
                radius = int(radius)
                
                print(f"[ROBOT] ✓ Balle {couleur} détectée : ({cx}, {cy}), rayon: {radius}")
                
                # Dessin pour debug
                cv2.circle(frame, (cx, cy), radius, (0, 255, 0), 2)
                cv2.circle(frame, (cx, cy), 5, (0, 0, 255), -1)
                
                # Sauvegarde avec détection
                cv2.imwrite(chemin_detectee, frame)
                print(f"[ROBOT] 📸 Photo avec détection sauvegardée: {chemin_detectee}")
                
                cap.release()
                return (cx, cy, radius)
            else:
                print("[ROBOT] ⚠ Objet trop petit")
        else:
            print(f"[ROBOT] ⚠ Aucune balle {couleur} détectée")
    else:
        print("[ROBOT] ❌ Erreur capture caméra")
    
    cap.release()
    return None


def envoyer_position_balle(cx, cy, frame_width=640, frame_height=480):
    """
    Envoie des commandes au robot en fonction de la position de la balle.
    - Si au centre : envoyer '1' (5s - avancer)
    - Si à gauche : envoyer '4' (0.2s - tourner gauche)
    - Si à droite : envoyer '5' (0.2s - tourner droite)
    """
    # Diviser l'écran en 3 zones horizontales (gauche, centre, droite)
    zones_x = frame_width // 3
    
    col = cx // zones_x  # 0=gauche, 1=centre, 2=droite
    
    if col == 1:  # Balle au centre
        print(f"[ROBOT] Balle au CENTRE (x={cx}) → '1' pendant 5s")
        envoyer_arduino('1', duree=5)
    elif col == 0:  # Balle à gauche
        print(f"[ROBOT] Balle à GAUCHE (x={cx}) → '4' pendant 0.2s")
        envoyer_arduino('4', duree=0.2)
    else:  # col == 2, balle à droite
        print(f"[ROBOT] Balle à DROITE (x={cx}) → '5' pendant 0.2s")
        envoyer_arduino('5', duree=0.2)
    
    # Reprendre une photo après le mouvement
    time.sleep(0.5)
    print("[ROBOT] Reprise de photo...\n")




def main():
    print("[ROBOT] Lecture des actions en cours...")
    actions = lire_actions()

    if not actions:
        print("[ROBOT] Aucune action à exécuter.")
        return

    for ligne in actions:
        parts = ligne.split()
        action = parts[0]

        if action == "advance":
            distance = parts[1]
            duree=(int((distance)-0.65)/0.77)+1
            envoyer_arduino('1', duree) # '1' pour avancer
        elif action == "retreat":
            distance = parts[1]
            duree=(int((distance)-0.65)/0.77)+1
            envoyer_arduino('2', duree) # '2' pour reculer
        elif action == "turn":
            envoyer_arduino('6', duree=0.1) 
            if parts[1] == "left":
                envoyer_arduino('4', duree=1.2) # '4' pour gauche
            else:
                envoyer_arduino('5', duree=1.2) # '5' pour droite
            envoyer_arduino('7', duree=0.1)
        elif action == "find_ball":
            # Récupérer la couleur de la balle
            couleur = parts[1] if len(parts) > 1 else "red"
            print(f"[ROBOT] Recherche balle {couleur}...")
            
            max_iterations = 10  # Limite pour éviter boucle infinie
            iteration = 0
            balle_centree = False
            
            while iteration < max_iterations and not balle_centree:
                iteration += 1
                print(f"[ROBOT] Itération {iteration}/{max_iterations}")
                
                # Détecter la balle
                resultat = detecter_balle(couleur)
                
                if resultat:
                    cx, cy, radius = resultat
                    
                    # Vérifier si la balle est au centre
                    zones_x = 640 // 3
                    col = cx // zones_x
                    
                    if col == 1:  # Balle au centre
                        print(f"[ROBOT] ✓ Balle {couleur} trouvée et centrée!")
                        envoyer_position_balle(cx, cy)
                        balle_centree = True
                    else:  # Balle pas au centre
                        # Envoyer la position et faire un mouvement
                        envoyer_position_balle(cx, cy)
                else:
                    print(f"[ROBOT] Impossible de détecter la balle {couleur}")
                    envoyer_arduino('0', duree=0.1)  # Code d'erreur
                    break
            
            if not balle_centree and iteration >= max_iterations:
                print(f"[ROBOT] Nombre d'itérations atteint ({max_iterations})")

        elif action == "stop":
            envoyer_arduino('3', duree=0.1) # '3' pour stop
            break

    # Nettoyage du fichier action.txt
    open(FICHIER_ACTION, "w").close()
    print("[ROBOT] Toutes les actions ont été effectuées.")

if __name__ == "__main__":
    main()
import turtle
import time
import os
from robot import Robot   # classe Robot 

# Configuration générale


FICHIER_ACTION = "action.txt"

# Mouvement global (une action)
DISTANCE = 80      # distance totale pour AVANCE
ANGLE = 90         # angle total pour GAUCHE / DROITE

# Animation progressive
PAS_DISTANCE = 5   # déplacement par pas
PAS_ANGLE = 5      # rotation par pas
DELAI_ANIM = 0.02  # délai entre chaque pas (plus petit = plus rapide)

DELAI_LECTURE = 0.1  # délai entre deux lectures de action.txt



def lire_action():
    """Lit l'action envoyée par le programme C."""
    if not os.path.exists(FICHIER_ACTION):
        print("fichier n'existe pas")
        return None

    with open(FICHIER_ACTION, "r") as f:
        action = f.read().strip()

    return action if action else None


def effacer_action():
    """Efface l'action après traitement."""
    open(FICHIER_ACTION, "w").close()


def appliquer_action(robot, action):
    """
    Applique une action avec animation progressive.
    Retourne False si la simulation doit s'arrêter.
    """

    if action == "avance":
        distance_restante = DISTANCE
        while distance_restante > 0:
            pas = min(PAS_DISTANCE, distance_restante)
            robot.avancer(pas)
            distance_restante -= pas
            time.sleep(DELAI_ANIM)

    elif action == "gauche":
        angle_restant = ANGLE
        while angle_restant > 0:
            pas = min(PAS_ANGLE, angle_restant)
            robot.tourner_gauche(pas)
            angle_restant -= pas
            time.sleep(DELAI_ANIM)

    elif action == "droite":
        angle_restant = ANGLE
        while angle_restant > 0:
            pas = min(PAS_ANGLE, angle_restant)
            robot.tourner_droite(pas)
            angle_restant -= pas
            time.sleep(DELAI_ANIM)

    elif action == "stop":
        print("[SIMULATION] Arrêt demandé")
        return False

    else:
        print(f"[SIMULATION] Action inconnue : {action}")

    return True




def main():
    screen = turtle.Screen()
    screen.title("Simulation Robot – Commande Vocale")
    screen.bgcolor("white")

    robot = Robot()
    print("[SIMULATION] En attente de commandes vocales...")

    en_cours = True

    while en_cours:
        action = lire_action()

        if action:
            print(f"[SIMULATION] Action reçue : {action}")
            en_cours = appliquer_action(robot, action)
            effacer_action()   

        time.sleep(DELAI_LECTURE)

    turtle.exitonclick

if __name__ == "__main__":
    main()
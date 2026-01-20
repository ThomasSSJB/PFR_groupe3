import turtle
import time
from robot import Robot   # classe Robot 

# Configuration générale


f= open("action.txt","r")
FICHIER_ACTION = "action.txt"


# Mouvement global (une action)
DISTANCE = 80      # distance totale pour AVANCE
ANGLE = 90         # angle total pour GAUCHE / DROITE

# Animation progressive
PAS_DISTANCE = 5   # déplacement par pas
PAS_ANGLE = 5      # rotation par pas
DELAI_ANIM = 0.02  # délai entre chaque pas (plus petit = plus rapide)

DELAI_LECTURE = 0.1  # délai entre deux lectures de action.txt

def decouper_commandes(mots):
    commandes = []
    courante = []

    for mot in mots:
        if mot == "and":
            if courante:
                commandes.append(courante)
                courante = []
        else:
            courante.append(mot)

    if courante:
        commandes.append(courante)

    return commandes


def lire_action():
    """Lit l'action envoyée par le programme C."""
    with open(FICHIER_ACTION, "r", encoding="utf-8") as f:
        mots = f.read().split()

    print(mots)
    return mots


def effacer_action():
    """Efface l'action après traitement."""
    open(FICHIER_ACTION, "w").close()


def tourner(robot, mots):
    """Gère l'action tourner avec direction et angle."""
    direction = None
    if "left" in mots or "gauche" in mots:
        direction = "left"
    elif "right" in mots or "droite" in mots:
        direction = "right"
    if direction:
        angle = ANGLE
        for word in mots:
            if word.isdigit():
                angle = int(word)
                break
        angle_restant = angle
        while angle_restant > 0:
            pas = min(PAS_ANGLE, angle_restant)
            robot.tourner(pas, direction)
            angle_restant -= pas
            time.sleep(DELAI_ANIM)


def appliquer_actions(mots, robot):
    commandes = decouper_commandes(mots)

    for cmd in commandes:
        action = cmd[0]
        params = cmd[1:]

        if action == "advance":
            params = cmd[1:]
            numbers = [int(p) for p in params if p.lstrip("-").isdigit()]

            if "to" in params and len(numbers) >= 2:
                robot.aller_a(numbers[0], numbers[1])
            else:
                distance = DISTANCE
                if numbers:
                    distance = numbers[0]
                robot.avancer(distance)



        elif action == "retreat":
            distance = DISTANCE
            for p in params:
                if p.isdigit():
                    distance = int(p)
                    break
            robot.reculer(distance)

        elif action == "turn":
            direction = None
            if "left" in params or "gauche" in params:
                direction = "left"
            elif "right" in params or "droite" in params:
                direction = "right"

            if direction:
                angle = ANGLE
                for p in params:
                    if p.isdigit():
                        angle = int(p)
                        break
                robot.tourner(angle, direction)

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
            en_cours = appliquer_actions(action, robot)
            effacer_action()   

        time.sleep(DELAI_LECTURE)

    turtle.exitonclick()

if __name__ == "__main__":
    main()
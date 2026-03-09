import turtle
import time
import os
import math
from robot import Robot
import environnement

# ======================================================
# CHEMIN VERS action.txt
# ======================================================

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
FICHIER_ACTION = os.path.join(BASE_DIR, "..", "data/action.txt")

# ======================================================
# PARAMÈTRES
# ======================================================

PAS_DISTANCE = 5
PAS_ANGLE = 5
DELAI_ANIM = 0.02

# ======================================================
# MAPPING COULEURS (VOIX → ENVIRONNEMENT)
# ======================================================

COLOR_MAP = {
    "red": "rouge",
    "blue": "bleu",
    "yellow": "jaune",
    "green": "vert"
}

# ======================================================
# LECTURE ACTIONS
# ======================================================

def lire_actions():
    if not os.path.exists(FICHIER_ACTION):
        return []
    with open(FICHIER_ACTION, "r", encoding="utf-8") as f:
        return [l.strip() for l in f if l.strip()]

def effacer_actions():
    open(FICHIER_ACTION, "w").close()

# ======================================================
# MOUVEMENTS
# ======================================================

def avancer_progressif(robot, env, distance, sens=1):
    reste = distance
    while reste > 0 and not detecter_mur(robot, env):
        pas = min(PAS_DISTANCE, reste)
        robot.t.forward(pas if sens == 1 else -pas)
        reste -= pas
        time.sleep(DELAI_ANIM)

def tourner_progressif(robot, angle, direction):
    reste = angle
    while reste > 0:
        pas = min(PAS_ANGLE, reste)
        if direction == "left":
            robot.t.left(pas)
        else:
            robot.t.right(pas)
        reste -= pas
        time.sleep(DELAI_ANIM)

def detecter_ouverture(robot, env):
    rx, ry = robot.t.position()
    for ouv in env["ouvertures"]:
        ouvx, ouvy = ouv["pos_porte"]
        if ((abs(rx - ouvx) < 25) or (abs(ry - ouvy) < 25)):
            return True
    return False

def detecter_mur(robot, env):
    marge = 40
    
    rx, ry = robot.t.position()
    envx, envy = env["dimensions"]

    envx //= 2
    envy //= 2

    if (rx > (envx-marge) or rx < (-envx+marge) or ry > (envy-marge) or ry < (-envy+marge)) and not detecter_ouverture(robot, env):
        print("[SIMULATION] Déplacement impossible : mur en approche !")
        return True
    else:
        return False


# ======================================================
# FIND BALL (CORRIGÉ)
# ======================================================

def chercher_balle(robot, env, couleur=None):
    print("[SIMULATION] Recherche de balle")

    balles = env.get("obstacles", [])

    # print("[SIMULATION - TRACE] balles =", balles)

    if not balles:
        print("[SIMULATION] Aucune balle dans l'environnement")
        return

    # Traduction couleur EN → FR
    # print("[SIMULATION - TRACE] AVANT -> couleur =", couleur)
    # if couleur:
    #     couleur = COLOR_MAP.get(couleur, couleur)
    # print("[SIMULATION - TRACE] APRES -> couleur =", couleur)

    rx, ry = robot.t.position()
    cible = None
    dist_min = float("inf")

    for b in balles:
        nom = b["nom"]          # ex: balle_rouge
        bx, by = b["centre"]

        bx += 15
        by -= 15

        if couleur and couleur not in nom:
            continue

        dist = math.hypot(bx - rx, by - ry) - 30

        if dist < dist_min:
            dist_min = dist
            cible = b

    if not cible:
        print("[SIMULATION] Aucune balle correspondante")
        return

    bx, by = cible["centre"]

    bx += 15
    by -= 15

    dx = bx - rx
    dy = by - ry

    angle = math.degrees(math.atan2(dy, dx))
    robot.t.setheading(angle)

    avancer_progressif(robot, env, int(dist_min))

    robot.t.dot(14, cible["couleur"])
    print(f"[SIMULATION] Balle atteinte : {cible['nom']}")

# ======================================================
# APPLICATION ACTION
# ======================================================

def appliquer_action(ligne, robot, env):
    parts = ligne.split()
    action = parts[0]

    if action == "advance":
        if len(parts) > 2 and parts[1] == "to":
            robot.aller_a(int(parts[2]), int(parts[3]))
        else:
            avancer_progressif(robot, env, int(parts[1]))

    elif action == "retreat":
        avancer_progressif(robot, env, int(parts[1]), sens=-1)

    elif action == "turn":
        tourner_progressif(robot, int(parts[2]), parts[1])

    elif action == "find_ball":
        if len(parts) > 1:
            chercher_balle(robot, env, parts[1])
        else:
            chercher_balle(robot, env)

    elif action == "stop":
        return False

    else:
        print(f"[SIMULATION] Action inconnue : {ligne}")

    robot.t.dot(6, "red")
    return True

# ======================================================
# MAIN
# ======================================================

def main(env):
    screen = turtle.Screen()
    screen.title("Simulation Robot – PFR")
    screen.bgcolor("white")
    screen.tracer(0)

    environnement.tracer_environnement(env)

    robot = Robot(start_x=0, start_y=-250, initial_heading=90)

    print("\n[SIMULATION] Lecture des actions...")
    actions = lire_actions()

    if not actions:
        print("[SIMULATION] Aucune action")
        turtle.mainloop()
        return

    for act in actions:
        if not appliquer_action(act, robot, env):
            break
        screen.update()

    effacer_actions()
    print("[SIMULATION] Actions terminées")

    turtle.mainloop()


if __name__ == "__main__":
    env = environnement.initialiser_environnement()
    main(env)

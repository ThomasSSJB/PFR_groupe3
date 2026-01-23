import turtle
import time
from robot import Robot
import simul
import simulation

def main():
    # 1. Créer la fenêtre turtle UNE SEULE FOIS
    screen = turtle.Screen()
    screen.title("Simulation Robot - Environnement complet")
    screen.bgcolor("white")
    
    # 2. Tracer l'environnement avec simul.py
    env = simul.initialiser_environnement()
    simul.tracer_environnement(env, clear_screen=False)
    
    # 3. Créer le robot
    robot = Robot(start_x=0, start_y=-300, initial_heading=180)
    
    # 4. Lire et exécuter les commandes (logique de simulation.py)
    FICHIER_ACTION = simulation.FICHIER_ACTION
    DELAI_LECTURE = simulation.DELAI_LECTURE
    
    print("[SIMULATION] En attente de commandes...")
    en_cours = True
    
    while en_cours:
        try:
            with open(FICHIER_ACTION, "r", encoding="utf-8") as f:
                contenu = f.read().strip()
            
            if contenu:
                mots = contenu.split()
                print(f"[SIMULATION] Action reçue : {mots}")
                en_cours = simulation.appliquer_actions(mots, robot)
                
                # Effacer le fichier
                with open(FICHIER_ACTION, "w") as f:
                    f.write("")
        except FileNotFoundError:
            pass
        except Exception as e:
            print(f"[ERREUR] {e}")
        
        time.sleep(DELAI_LECTURE)
    
    turtle.hideturtle()
    turtle.exitonclick()

if __name__ == "__main__":
    main()
# =======================EN TÊTE=====================================
# FICHIER: test_cas_utilisation.py
# AUTEUR: BACHAA Hajar
# RÔLE: Tests des cas d'utilisation pour la simulation de déplacement du robot.
# ===================================================================

import turtle
import json
from robot import Robot # Importez la classe Robot

# 1. Configuration de l'écran (le monde du robot)
def setup_simulation_screen(config):
	screen = turtle.Screen()
	#on recupere les dimensions depuis le JSON
	largeur=config['simulation_graphique']['largeur_ecran']
	hauteur=config['simulation_graphique']['hauteur_ecran']
	screen.setup(width=largeur, height=hauteur)
	screen.title("Simulation de Déplacement Robot (PFR1)")
	return screen

# DÉFINITION DES CAS DE DÉPLACEMENT

def cas_1_ligne_droite(robot: Robot, m):
	"""Cas 1 : Mouvement simple pour tester 'avancer' et 'tourner'."""
	print(m.get('case_1_title',"CAS 1"))
	robot.t.color("red")
	robot.avancer(100)
	robot.tourner_droite(90)
	robot.avancer(50)
	robot.ecrire_message(m.get('case_1_done', "Fin cas 1"))
	robot.aller_a(0, 0) # Retour à l'origine pour le cas suivant
	robot.t.setheading(0)


def cas_2_parcours_carre(robot: Robot, m):
	"""Cas 2 : Exécution d'un parcours prédéfini (Carré)."""
	print(m.get('case_2_title', "CAS 2"))
	robot.t.color("green")
	robot.t.setheading(90) # Regarder vers le Nord

	for i in range(4):
		robot.avancer(150)
		robot.tourner_droite(90)

	robot.ecrire_message(m.get('square_done', "carré effectué"))
	robot.aller_a(0, 0)
	robot.t.setheading(0)


def cas_3_atteindre_cible(robot: Robot, m,  x_target=150, y_target=100):
	"""Cas 3 : Déplacement vers des coordonnées spécifiques (Simulation de la détection)."""
	print(m.get('case_3_title', "CAS 3"))
	robot.t.color("violet")

	# Simuler la cible
	robot.t.penup()
	robot.t.goto(x_target, y_target)
	robot.marquer_cible(taille=15, couleur="violet")

	# Retourner à la position de départ
	robot.aller_a(0, 0)

	# Déplacement vers la cible
	robot.t.pendown() # On veut tracer le chemin
	robot.aller_a(x_target, y_target)

	robot.ecrire_message(m.get('target_reached', "Cible atteinte"))
	robot.aller_a(0, 0)
	robot.t.setheading(0)


def cas_4_gestion_erreur_obstacle(robot: Robot, m):
	"""Cas 4 : Simule un mouvement avec arrêt et redirection en cas d'obstacle."""
	print(m.get('case_4_title', "CAS 4"))

	robot.avancer(150)

	# Simulation de la détection d'un obstacle
	robot.marquer_cible(taille=20, couleur="orange")
	robot.t.color("orange")

	# Action de contournement/redirection
	robot.ecrire_message(m.get('obstacle_detected', "ERREUR: Obstacle"))
	robot.tourner_gauche(45)
	robot.avancer(50)

	robot.aller_a(0, 0)
	robot.t.setheading(0)



# EXÉCUTION PRINCIPALE
if __name__ == "__main__":

	# 1. Chargement de la configuration
	with open('../config/config.json', 'r', encoding='utf-8') as f:
		config = json.load(f)

	# 2. Extraction des réglages du JSON
	vitesse_config = config['robot_mouvement']['vitesse_defaut']
	couleur_config = config['robot_mouvement']['couleur_defaut']

	# Chargement du fichier de langue
	langue = config['parametres_generaux']['langue_actuelle']
	with open(f'../config/{langue}.json', 'r', encoding='utf-8') as f_lang:
		textes = json.load(f_lang)
		m = textes['messages']

	# 3. Initialisation
	screen = setup_simulation_screen(config)

	# Utilisation des variables du JSON pour créer le robot
	mon_robot = Robot(vitesse=vitesse_config, couleur=couleur_config)
	mon_robot.messages = m

	# 4. Exécution des cas
	cas_1_ligne_droite(mon_robot, m)
	turtle.time.sleep(1) 

	cas_2_parcours_carre(mon_robot, m)
	turtle.time.sleep(1)

	cas_3_atteindre_cible(mon_robot, m, 150, -150)
	turtle.time.sleep(1)

	cas_4_gestion_erreur_obstacle(mon_robot, m)

	screen.mainloop()

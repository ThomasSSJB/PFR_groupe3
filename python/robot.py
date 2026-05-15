# =======================EN TÊTE=====================================
# FICHIER: robot.py
# AUTEURS: BACHAA Hajar, DEVAUD Antoine
# RÔLE: Définition de la classe Robot
#       Gère les mouvements, l'affichage graphique et
#       l'interaction multilingue du robot.
# ===================================================================


import turtle
import math

class Robot:
	"""Initialisation du robot avec sa position, sa couleur et son orientation."""
	def __init__(self, start_x=0, start_y=0, initial_heading=0, couleur="blue", vitesse=1):

		#initialisation de la tortue
		self.t=turtle.Turtle()
		self.t.shape("turtle")
		self.t.speed(vitesse)
		self.t.color(couleur)

		#position initial
		self.t.penup()
		self.t.goto(start_x, start_y)
		self.t.setheading(initial_heading) # 0=EST, 90=NORT
		self.t.pendown()

		# Dictionnaire de messages (sera rempli par simulation.py)
		self.messages = {}

	def avancer(self, distance):
		"""Déplace le robot vers l'avant et logue l'action."""
		prefixe = self.messages.get('prefixe', '[TRACE]')
		action = self.messages.get('avancer', 'avance')
		unite = self.messages.get('unite_distance', 'metres')
		print(f"{prefixe} Robot {action} de {distance} {unite}.")
		self.t.forward(distance)

	def reculer(self, distance):
		"""Déplace le robot vers l'arrière et logue l'action."""
		prefixe = self.messages.get('prefixe', '[TRACE]')
		action = self.messages.get('reculer', 'recule')
		unite = self.messages.get('unite_distance', 'metres')
		print(f"{prefixe} Robot {action} de {distance} {unite}.")
		self.t.backward(distance)


	def tourner (self, angle,direction):
		prefixe = self.messages.get('prefixe', '[TRACE]')
		action = self.messages.get('tourner', 'tourne')
		unite = self.messages.get('unite_angle', 'degres')
		if direction=="right":
			print(f"{prefixe} Robot {action} de {angle} {unite} a droite.")
			self.t.right(angle)
		elif direction=="left":
			print(f"{prefixe} Robot {action} de {angle} {unite} a gauche.")
			self.t.left(angle)

	def stop(self):
		"""Lève le crayon (mode sans traçage)."""
		prefixe = self.messages.get('prefixe', '[TRACE]')
		# Note : Ajoute 'stop_trace' dans tes JSON pour traduire ce message
		print(f"{prefixe} Robot arrete de tracer (penup).")
		self.t.penup()

	def start(self):
		"""Abaisse le crayon (mode traçage)."""
		prefixe = self.messages.get('prefixe', '[TRACE]')
		# Note : Ajoute 'start_trace' dans tes JSON pour traduire ce message
		print(f"{prefixe} Robot commence a tracer (pendown).")
		self.t.pendown()

	def aller_a(self, x, y):
		"""Déplace le robot directement aux coordonnées (x,y)"""
		prefixe = self.messages.get('prefixe', '[TRACE]')
		action = self.messages.get('aller_a', 'se deplace a la position absolue')
		print(f"{prefixe} Robot {action} ({x}, {y}).")
		self.t.penup()
		self.t.goto(x, y)
		self.t.pendown()

	def marquer_cible(self, taille=10, couleur="red"):
		"""marque un point pour symboliser une cible ou un obstacle"""
		self.t.dot(taille, couleur)

	def ecrire_message(self, message):
		"""Écrit un message sur l'écran de simulation."""
		self.t.write(message, align="center", font=("Arial", 10, "normal"))

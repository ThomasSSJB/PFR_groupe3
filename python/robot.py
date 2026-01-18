import turtle

class Robot:

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
		prefix = self.messages.get('prefix', '[TRACE]')
		action = self.messages.get('advance', 'avance')
		unit = self.messages.get('distance_unit', 'metres')
		linker = self.messages.get('unit_linker', 'de') 
		print(f"{prefix} Robot {action} {linker} {distance} {unit}.")
		self.t.forward(distance)

	def reculer(self, distance):
		"""Déplace le robot vers l'arrière et logue l'action."""
		prefix = self.messages.get('prefix', '[TRACE]')
		action = self.messages.get('retreat', 'recule')
		unit = self.messages.get('distance_unit', 'metres')
		linker = self.messages.get('unit_linker', 'de') 
		print(f"{prefix} Robot {action} {linker} {distance} {unit}.")
		self.t.backward(distance)

	def tourner_droite(self, angle):
		prefix = self.messages.get('prefix', '[TRACE]')
		action = self.messages.get('turn_right', 'tourne a droite')
		unit = self.messages.get('angle_unit', 'degres')
		linker = self.messages.get('unit_linker', 'de') 
		print(f"{prefix} Robot {action} {linker} {angle} {unit}.")
		self.t.right(angle)

	def tourner_gauche(self, angle):
		prefix = self.messages.get('prefix', '[TRACE]')
		action = self.messages.get('turn_left', 'tourne a gauche')
		unit = self.messages.get('angle_unit', 'degres')
		linker = self.messages.get('unit_linker', 'de') 
		print(f"{prefix} Robot {action} {linker} {angle} {unit}.")
		self.t.left(angle)

	def stop(self):
		"""Lève le crayon (mode sans traçage)."""
		prefix = self.messages.get('prefix', '[TRACE]')
		action = self.messages.get('stop_trace', 'arrête de tracer')
		print(f"{prefix} Robot {action} (penup).")
		self.t.penup()

	def start(self):
		"""Abaisse le crayon (mode traçage)."""
		prefix = self.messages.get('prefixe', '[TRACE]')
		action = self.messages.get('start_trace', 'commence à tracer')
		print(f"{prefix} Robot {action} (pendown).")
		self.t.pendown()

	def aller_a(self, x, y):
		"""Déplace le robot directement aux coordonnées (x,y)"""
		prefix = self.messages.get('prefix', '[TRACE]')
		action = self.messages.get('go_to', 'va à')
		print(f"{prefix} Robot {action} ({x}, {y}).")
		self.t.penup()
		self.t.goto(x, y)
		self.t.pendown()

	def marquer_cible(self, taille=10, couleur="red"):
		"""marque un point pour symboliser une cible ou un obstacle"""
		self.t.dot(taille, couleur)

	def ecrire_message(self, message):
		"""Écrit un message sur l'écran de simulation."""
		self.t.write(message, align="center", font=("Arial", 10, "bold"))


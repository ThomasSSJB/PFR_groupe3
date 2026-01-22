import turtle as tl

# 1. GESTION DE L'ENVIRONNEMENT (OUVERTURES & OBSTACLES)

def ajouter_ouverture(piece, ouverture):
    if not isinstance(piece, dict) or not isinstance(ouverture, dict):
        return
    piece['ouvertures'].append(ouverture)

def ajouter_obstacle(piece, obstacle):
    if not isinstance(piece, dict) or not isinstance(obstacle, dict):
        return
    piece['obstacles'].append(obstacle)

# 2. TRACAGE

def tracer_piece(piece, couleur="purple", trait=5):
    L, l = piece['dimensions']
    x_HD, y_HD = piece['coin_HD']
    tl.up()
    tl.goto(x_HD, y_HD)
    tl.setheading(0)
    tl.down()
    tl.color(couleur)
    tl.width(trait)
    for _ in range(2):
        tl.right(90)
        tl.forward(l)
        tl.right(90)
        tl.forward(L)
    tl.up()

def tracer_ouvertures_piece(piece):
    ouvs = piece.get('ouvertures', [])
    x_HD, y_HD = piece['coin_HD']
    for ouv in ouvs:
        tl.up()
        tl.goto(x_HD, y_HD)
        tl.setheading(270)
        tl.forward(ouv['distance_porte_angle'])
        tl.down()
        tl.width(ouv['trait'])
        tl.color(ouv['couleur'])
        tl.forward(ouv['largeur'])
        tl.up()
        ouv['pos_porte'] = tl.pos()

def tracer_obstacles_piece(piece):
    obstacles = piece.get('obstacles', [])
    for obs in obstacles:
        x, y = obs['centre']
        dim = obs['dimension']
        tl.up()
        tl.width(obs['trait'])
        tl.color(obs['couleur'])
        if obs['type'] == 'cercle':
            tl.goto(x, y - dim)
            tl.down()
            tl.begin_fill()
            tl.circle(dim)
            tl.end_fill()
        elif obs['type'] == 'carré':
            tl.goto(x - dim/2, y + dim/2)
            tl.setheading(0)
            tl.down()
            tl.begin_fill()
            for _ in range(4):
                tl.forward(dim)
                tl.right(90)
            tl.end_fill()
        tl.up()

def tracer_environnement(piece, clear_screen=True):
    if clear_screen:
        tl.clearscreen()
    tl.speed(0)
    tracer_piece(piece)
    tracer_ouvertures_piece(piece)
    tracer_obstacles_piece(piece)

# 3. GESTION DU ROBOT ET DES TRAJECTOIRES

def ajouter_trajectoire(robot, trajectoire):
    if 'trajectoire' not in robot:
        robot['trajectoire'] = []
    robot['trajectoire'].extend(trajectoire)

def tracer_trajectoire_robot(robot):
    trajectoire = robot['trajectoire']
    if not trajectoire: return
    tl.color(robot['couleur'])
    tl.width(robot['trait'])
    tl.up()
    # On place le robot à sa position de départ réelle avant de tracer
    tl.goto(robot['pos_robot'])
    tl.down()
    
    for i, segment in enumerate(trajectoire):
        (x, y), orientation, distance, _ = segment
        # Le robot s'oriente : 0:Est, 1:Nord, 2:Ouest, 3:Sud
        tl.setheading(orientation * 90)
        tl.forward(distance)
    tl.up()

def modifier_orientation(ref_robot, nouvelle_orientation):
    ref_robot['sens'] = nouvelle_orientation
    return ref_robot

def executer_trajectoire(ref_robot, ref_env):
    trajectoire = ref_robot['trajectoire']
    for segment in trajectoire:
        (x, y), orientation, distance, _ = segment
        modifier_orientation(ref_robot, orientation)
        x_c, y_c = ref_robot['pos_robot']
        if orientation == 0: x_c += distance
        elif orientation == 1: y_c += distance
        elif orientation == 2: x_c -= distance
        elif orientation == 3: y_c -= distance
        ref_robot['pos_robot'] = (x_c, y_c)
    return ref_robot

# 4. INITIALISATION ET MISSION

def initialiser_environnement():
    piece = {'nom': 'entrepot', 'dimensions': (400, 400), 'coin_HD': (200, 200),
             'couleur': 'blue', 'trait': 5, 'ouvertures': [], 'obstacles': []}
    
    ouverture = {'nom': 'porte1', 'largeur': 60, 'distance_porte_angle': 100, 
                 'couleur': 'orange', 'trait': 10, 'pos_porte': ()}
    ajouter_ouverture(piece, ouverture)

    # Ajout des balles colorées
    ajouter_obstacle(piece, {'nom': 'balle_rouge', 'type': 'cercle', 'centre': (-50, 50), 
                             'dimension': 15, 'couleur': 'red', 'trait': 1})
    ajouter_obstacle(piece, {'nom': 'balle_jaune', 'type': 'cercle', 'centre': (0, -80), 
                             'dimension': 20, 'couleur': 'yellow', 'trait': 1})
    ajouter_obstacle(piece, {'nom': 'balle_verte', 'type': 'cercle', 'centre': (100, 40), 
                             'dimension': 10, 'couleur': 'green', 'trait': 1})
    return piece

def definir_mission_robot(ref_env):
    # Position de départ calculée selon la porte
    ouv = ref_env['ouvertures'][0]
    # Simulation du tracé pour obtenir pos_porte si non définie
    start_x = 200 # mur droit
    start_y = 200 - 100 - (60/2) # milieu de la porte
    
    robot = {'nom': 'r2d2', 'couleur': 'darkred', 'trait': 3, 
             'pos_robot': (start_x, start_y), 'sens': 2, 'trajectoire': []}
    
    # Nouvelle trajectoire pour slalomer entre les balles
    # Format : [(pos_debut), orientation, distance, info]
    trajectoire_slalom = [
        [(start_x, start_y), 2, 100, ()],  # Va vers la gauche
        [(100, start_y), 1, 100, ()],      # Monte vers la balle verte
        [(100, 140), 2, 150, ()],          # Passe au dessus de la rouge
        [(-50, 140), 3, 200, ()],          # Descend vers la jaune
        [(-50, -60), 0, 150, ()]           # Finit vers la droite
    ]
    
    ajouter_trajectoire(robot, trajectoire_slalom)
    return robot

# 5. PROGRAMME PRINCIPAL

# 1. Créer l'univers
env = initialiser_environnement()

# 2. Dessiner l'univers (murs, porte, balles)
tracer_environnement(env)

# 3. Préparer le robot et sa route
robot = definir_mission_robot(env)

# 4. Tracer le mouvement sur l'écran
tracer_trajectoire_robot(robot)

# 5. Mettre à jour les données du robot (calcul interne)
executer_trajectoire(robot, env)

print(f"Mission accomplie. Position finale de {robot['nom']} : {robot['pos_robot']}")

tl.hideturtle()
tl.exitonclick()

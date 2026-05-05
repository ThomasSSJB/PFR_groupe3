import open3d as o3d
import numpy as np
import math
import sys

# --- CONFIGURATION ---
LIMIT = 5000       # Rayon max d'affichage (5m)
MAX_POINTS = 20000 # Nombre de points stockés en mémoire
UPDATE_EVERY = 50  # Rafraîchir l'écran tous les X points

def main():
    # 1. Initialisation d'Open3D
    pcd = o3d.geometry.PointCloud()
    vis = o3d.visualization.Visualizer()
    vis.create_window(window_name="PFR3 - LiDAR Live Map", width=1024, height=768)
    
    # Ajout d'un repère (RGB = XYZ) et d'un nuage de points vide
    grid = o3d.geometry.TriangleMesh.create_coordinate_frame(size=500)
    vis.add_geometry(grid)
    vis.add_geometry(pcd)
    
    # Options de rendu (points plus gros et fond sombre)
    opt = vis.get_render_option()
    opt.point_size = 2.0
    opt.background_color = np.asarray([0.1, 0.1, 0.1])

    all_points = []
    print("Réception des données... (Appuyez sur 'Q' dans la fenêtre pour quitter)")

    try:
        for line in sys.stdin:
            try:
                parts = line.strip().split(",")
                if len(parts) != 2: continue
                
                angle, distance = map(float, parts)

                # Filtrage et conversion Polaire -> Cartésien
                if 150 < distance < LIMIT:
                    theta = math.radians(angle)
                    x = distance * math.cos(theta)
                    y = distance * math.sin(theta)
                    all_points.append([x, y, 0]) # Z=0 pour un plan 2D

                # Gestion de la mémoire
                if len(all_points) > MAX_POINTS:
                    all_points = all_points[-MAX_POINTS:]

                # Mise à jour de l'affichage
                if len(all_points) % UPDATE_EVERY == 0:
                    pcd.points = o3d.utility.Vector3dVector(np.asarray(all_points))
                    vis.update_geometry(pcd)
                    vis.poll_events()
                    vis.update_renderer()

            except ValueError:
                continue

    except KeyboardInterrupt:
        pass
    finally:
        print("\nFermeture...")
        vis.destroy_window()

if __name__ == "__main__":
    main()
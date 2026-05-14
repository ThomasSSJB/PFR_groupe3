#!/usr/bin/env python3

# =======================EN TÊTE=====================================
# FICHIER: display_map.py
# AUTEUR: YAHYAOUI Nidal
# RÔLE: Cartographie par LiDAR et création d'une image résultat
# PRINCIPE:
#   - séparation murs / obstacles
#   - conservation de la géométrie réelle
#   - sauvegarde automatique dans data/lidar_map.png
#   - parsing corrigé angle,distance
#   - visualisation améliorée
# USAGE:
#   ssh pfr3@pfr3.local \
#   "source ~/code/PFR_groupe3/venv/bin/activate && \
#   python ~/code/PFR_groupe3/python/scan_lidar.py" \
#   | python display_map.py
# ===================================================================

import sys
import os
import math
import numpy as np
import cv2
import matplotlib

# Utiliser 'Agg' ou 'TkAgg' suivant l'OS
matplotlib.use('Agg')
# matplotlib.use('TkAgg')

import matplotlib.pyplot as plt
import matplotlib.patheffects as pe
from matplotlib.patches import Circle
import matplotlib.gridspec as gridspec

# ─────────────────────────────────────────────────────────────
# CONFIG
# ─────────────────────────────────────────────────────────────
DIST_MIN = 80
DIST_MAX = 6000

ANGLE_BIN_WIDTH    = 2.0
WALL_PERCENTILE    = 97
OBSTACLE_MAX_RATIO = 0.78

# Morphologie
WALL_DILATION = 3
WALL_CLOSE    = 4
WALL_MIN_AREA = 200

OBS_DILATION  = 2
OBS_CLOSE     = 3
OBS_MIN_AREA  = 40

IMG_SIZE = 1600
SCALE    = 0.11

# ─────────────────────────────────────────────────────────────
# LECTURE STDIN
# ─────────────────────────────────────────────────────────────
raw_pts = []

print("Lecture des points LiDAR...", file=sys.stderr)

for line in sys.stdin:

    line = line.strip()

    if not line:
        continue

    try:
        angle, dist = map(float, line.split(","))
    except:
        continue

    if not (DIST_MIN <= dist <= DIST_MAX):
        continue

    rad = math.radians(angle)

    x = dist * math.cos(rad)
    y = dist * math.sin(rad)

    raw_pts.append((angle, dist, x, y))

if len(raw_pts) == 0:
    print("Aucun point valide", file=sys.stderr)
    sys.exit(1)

angles = np.array([p[0] for p in raw_pts])
dists  = np.array([p[1] for p in raw_pts])
xs     = np.array([p[2] for p in raw_pts])
ys     = np.array([p[3] for p in raw_pts])

print(f"{len(xs)} points valides", file=sys.stderr)

# ─────────────────────────────────────────────────────────────
# SÉPARATION MURS / OBSTACLES
# ─────────────────────────────────────────────────────────────
bins = np.arange(0, 360 + ANGLE_BIN_WIDTH, ANGLE_BIN_WIDTH)

bin_idx = np.digitize(angles, bins) - 1

wall_dist = np.zeros(len(bins))

for b in range(len(bins)):

    mask = bin_idx == b

    if np.sum(mask) > 0:
        wall_dist[b] = np.percentile(
            dists[mask],
            WALL_PERCENTILE
        )

local_wall = wall_dist[
    np.clip(bin_idx, 0, len(wall_dist)-1)
]

is_wall = dists >= local_wall * OBSTACLE_MAX_RATIO
is_obs  = ~is_wall

wall_xs = xs[is_wall]
wall_ys = ys[is_wall]

obs_xs = xs[is_obs]
obs_ys = ys[is_obs]

print(f"Murs      : {len(wall_xs)} pts", file=sys.stderr)
print(f"Obstacles : {len(obs_xs)} pts", file=sys.stderr)

# ─────────────────────────────────────────────────────────────
# CONVERSION POINTS -> IMAGE
# ─────────────────────────────────────────────────────────────
center = IMG_SIZE // 2

def pts_to_img(xarr, yarr):

    img = np.zeros((IMG_SIZE, IMG_SIZE), dtype=np.uint8)

    for x, y in zip(xarr, yarr):

        px = int(center + x * SCALE)
        py = int(center - y * SCALE)

        if 0 <= px < IMG_SIZE and 0 <= py < IMG_SIZE:
            img[py, px] = 255

    return img

# ─────────────────────────────────────────────────────────────
# IMAGE MURS
# ─────────────────────────────────────────────────────────────
img_wall = pts_to_img(wall_xs, wall_ys)

k = np.ones((WALL_DILATION, WALL_DILATION), np.uint8)

img_wall = cv2.dilate(img_wall, k, iterations=1)

k2 = np.ones((WALL_CLOSE, WALL_CLOSE), np.uint8)

img_wall = cv2.morphologyEx(
    img_wall,
    cv2.MORPH_CLOSE,
    k2
)

contours_wall, _ = cv2.findContours(
    img_wall,
    cv2.RETR_EXTERNAL,
    cv2.CHAIN_APPROX_SIMPLE
)

contours_wall = [
    c for c in contours_wall
    if cv2.contourArea(c) > WALL_MIN_AREA
]

# ─────────────────────────────────────────────────────────────
# IMAGE OBSTACLES
# ─────────────────────────────────────────────────────────────
img_obs = pts_to_img(obs_xs, obs_ys)

ko = np.ones((OBS_DILATION, OBS_DILATION), np.uint8)

img_obs = cv2.dilate(img_obs, ko, iterations=1)

ko2 = np.ones((OBS_CLOSE, OBS_CLOSE), np.uint8)

img_obs = cv2.morphologyEx(
    img_obs,
    cv2.MORPH_CLOSE,
    ko2
)

contours_obs, _ = cv2.findContours(
    img_obs,
    cv2.RETR_EXTERNAL,
    cv2.CHAIN_APPROX_SIMPLE
)

contours_obs = [
    c for c in contours_obs
    if cv2.contourArea(c) > OBS_MIN_AREA
]

print(f"Contours murs      : {len(contours_wall)}", file=sys.stderr)
print(f"Contours obstacles : {len(contours_obs)}", file=sys.stderr)

# ─────────────────────────────────────────────────────────────
# FIGURE
# ─────────────────────────────────────────────────────────────
fig = plt.figure(figsize=(13, 11), facecolor='#02050e')

gs = gridspec.GridSpec(
    2,
    1,
    height_ratios=[14, 1],
    hspace=0.04
)

ax = fig.add_subplot(gs[0])
axb = fig.add_subplot(gs[1])

ax.set_facecolor('#030810')
axb.set_facecolor('#02050e')

limit = DIST_MAX

# ─────────────────────────────────────────────────────────────
# GRILLE
# ─────────────────────────────────────────────────────────────
for t in np.arange(-limit, limit + 1, 1000):

    ax.axhline(
        t,
        color='#0a1828',
        linewidth=0.4
    )

    ax.axvline(
        t,
        color='#0a1828',
        linewidth=0.4
    )

for r in range(1000, limit + 1, 1000):

    ax.add_patch(
        Circle(
            (0, 0),
            r,
            fill=False,
            edgecolor='#102030',
            linewidth=0.5,
            linestyle='--'
        )
    )

# ─────────────────────────────────────────────────────────────
# AFFICHAGE MURS
# ─────────────────────────────────────────────────────────────
for cnt in contours_wall:

    pts = cnt[:, 0, :]

    wx = (pts[:, 0] - center) / SCALE
    wy = -(pts[:, 1] - center) / SCALE

    ax.plot(
        wx,
        wy,
        color='#00d5ff',
        linewidth=2.2,
        zorder=5,
        path_effects=[
            pe.Stroke(
                linewidth=5,
                foreground='#003344',
                alpha=0.6
            ),
            pe.Normal()
        ]
    )

# ─────────────────────────────────────────────────────────────
# AFFICHAGE OBSTACLES
# ─────────────────────────────────────────────────────────────
for cnt in contours_obs:

    pts = cnt[:, 0, :]

    ox = (pts[:, 0] - center) / SCALE
    oy = -(pts[:, 1] - center) / SCALE

    ax.fill(
        ox,
        oy,
        color='#88cc00',
        alpha=0.35,
        zorder=6
    )

    ax.plot(
        ox,
        oy,
        color='#c8ff55',
        linewidth=1.2,
        zorder=7
    )

# ─────────────────────────────────────────────────────────────
# POINTS BRUTS
# ─────────────────────────────────────────────────────────────
ax.scatter(
    wall_xs,
    wall_ys,
    s=1,
    c='#00aaff',
    alpha=0.35,
    linewidths=0,
    zorder=3
)

ax.scatter(
    obs_xs,
    obs_ys,
    s=1,
    c='#99ff00',
    alpha=0.45,
    linewidths=0,
    zorder=4
)

# ─────────────────────────────────────────────────────────────
# ROBOT
# ─────────────────────────────────────────────────────────────
ax.add_patch(
    Circle(
        (0, 0),
        120,
        color='#ff2255',
        alpha=0.25,
        zorder=8
    )
)

ax.plot(
    0,
    0,
    'o',
    color='#ff2255',
    markersize=8,
    zorder=9
)

ax.text(
    0,
    -180,
    'ROBOT',
    color='#ff4466',
    fontsize=8,
    ha='center',
    fontweight='bold'
)

# ─────────────────────────────────────────────────────────────
# STYLE
# ─────────────────────────────────────────────────────────────
ax.set_xlim(-limit, limit)
ax.set_ylim(-limit, limit)

ax.set_aspect('equal')

ax.tick_params(
    colors='#335577',
    labelsize=7
)

for sp in ax.spines.values():
    sp.set_color('#112233')

ax.set_title(
    'LIDAR · ROOM RECONSTRUCTION',
    color='#00d5ff',
    fontsize=16,
    fontweight='bold'
)

# ─────────────────────────────────────────────────────────────
# BARRE INFO
# ─────────────────────────────────────────────────────────────
axb.axis('off')

info = (
    f"Points : {len(xs)}   |   "
    f"Min : {dists.min():.0f} mm   |   "
    f"Max : {dists.max():.0f} mm   |   "
    f"Murs : {len(contours_wall)}   |   "
    f"Obstacles : {len(contours_obs)}"
)

axb.text(
    0.5,
    0.5,
    info,
    transform=axb.transAxes,
    ha='center',
    va='center',
    color='#5588aa',
    fontsize=8,
    fontfamily='monospace'
)

# ─────────────────────────────────────────────────────────────
# SAUVEGARDE
# ─────────────────────────────────────────────────────────────
BASE_DIR = os.path.dirname(os.path.abspath(__file__))

SAVE_PATH = os.path.join(
    BASE_DIR,
    "..",
    "data",
    "lidar_map.png"
)

plt.savefig(
    SAVE_PATH,
    dpi=160,
    bbox_inches='tight',
    facecolor=fig.get_facecolor()
)

print(f"Carte sauvegardée : {SAVE_PATH}", file=sys.stderr)

plt.close()

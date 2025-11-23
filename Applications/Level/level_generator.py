from PIL import Image, ImageChops
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import math

# --- CONFIGURATION ---
INPUT_IMAGE = r"sprites\NES - Super Mario Bros. - Stages - World 1-1.png"
OUTPUT_FILENAME = "LevelCollision.h"
ARRAY_NAME = "collision_map"
TILE_SIZE = 16

# --- REGLAGES DEBUG ---
SHOW_DEBUG = True
SHOW_COORDS = False
# Taille du texte des coordonnées (tout petit pour tenir dans 16px)
FONT_SIZE = 5
GRID_COLOR = 'white'  # Couleur de la grille
GRID_ALPHA = 0.3      # Transparence de la grille

# --- CONFIGURATION DE DETECTION (Comme avant) ---
BACKGROUND_COLORS = [
    (92, 148, 252), (0, 0, 0), (255, 255, 255),
    (0, 168, 0), (128, 208, 16), (248, 56, 0)
]
COLOR_TOLERANCE = 15

# Modèles de tuyaux (A ajuster avec les coordonnées que tu verras sur l'écran ! SHOW_COORDS=True)
SOLID_TEMPLATES_COORDS = [
    (28, 11), (29, 11), (28, 12), (29, 12),  # tuyau
    (48, 17),  # brique sombre
    (63, 17), (61, 26), (61, 27), (62, 26), (62, 27), (63, 26), (63, 27),  # tuyau
    (20, 9),  # brique marron
]
MATCH_THRESHOLD = 50


def get_tile_image(img, tile_x, tile_y):
    x = tile_x * TILE_SIZE
    y = tile_y * TILE_SIZE
    return img.crop((x, y, x + TILE_SIZE, y + TILE_SIZE))


def compare_images(img1, img2):
    diff = ImageChops.difference(img1, img2)
    if not diff.getbbox():
        return 0
    h = diff.histogram()
    sq = (value * ((idx % 256) ** 2) for idx, value in enumerate(h))
    sum_of_squares = sum(sq)
    return math.sqrt(sum_of_squares / float(img1.size[0] * img1.size[1]))


def show_debug_view(img, collision_data, tiles_w, tiles_h):
    print("Génération de la vue Debug (cela peut prendre quelques secondes)...")

    # On crée une figure très large pour y voir clair
    fig, ax = plt.subplots(figsize=(20, 6))
    ax.imshow(img)

    # --- CORRECTION DU DECALAGE ICI ---
    # extent = [gauche, droite, bas, haut]
    # On force l'image à commencer exactement à 0,0 et finir à width,height
    # Note : l'axe Y est inversé en image (le haut est 0, le bas est height)
    ax.imshow(img, extent=[0, img.width, img.height, 0])

    print(f"Dessin de {tiles_w * tiles_h} tuiles...")

    for y in range(tiles_h):
        for x in range(tiles_w):
            # 1. Dessiner la grille pour TOUTES les tuiles
            # fill=False pour n'avoir que le contour
            grid_rect = patches.Rectangle(
                (x * TILE_SIZE, y * TILE_SIZE),
                TILE_SIZE, TILE_SIZE,
                linewidth=0.5,
                edgecolor=GRID_COLOR,
                facecolor='none',
                alpha=GRID_ALPHA
            )
            ax.add_patch(grid_rect)

            if SHOW_COORDS:
                # 2. Afficher les coordonnées (X, Y)
                # On écrit en blanc avec un fond noir (bbox) pour que ce soit lisible partout
                # Z-order élevé pour être sûr que le texte est au dessus du rouge
                ax.text(
                    x * TILE_SIZE + 8, y * TILE_SIZE + 8,  # Position Centre
                    f"{x}\n{y}",                          # Texte
                    color='cyan',
                    fontsize=FONT_SIZE,
                    ha='center', va='center',
                    fontweight='bold'
                )

            # 3. Dessiner le rouge SI Collision
            index = y * tiles_w + x
            if collision_data[index] == 1:
                col_rect = patches.Rectangle(
                    (x * TILE_SIZE, y * TILE_SIZE),
                    TILE_SIZE, TILE_SIZE,
                    linewidth=0,
                    facecolor='red',
                    alpha=0.4  # Assez transparent pour lire le texte dessous
                )
                ax.add_patch(col_rect)

    plt.title("Carte des collisions avec Coordonnées (Zoomez pour lire X,Y)")
    plt.tight_layout()
    plt.show()


def generate_collision_script():
    try:
        full_img = Image.open(INPUT_IMAGE).convert('RGB')
        tiles_w = full_img.width // TILE_SIZE
        tiles_h = full_img.height // TILE_SIZE

        # --- ETAPE 1 : Apprentissage ---
        templates = []
        for tx, ty in SOLID_TEMPLATES_COORDS:
            if tx < tiles_w and ty < tiles_h:
                templates.append(get_tile_image(full_img, tx, ty))

        collision_data = []

        # --- ETAPE 2 : Analyse ---
        for y in range(tiles_h):
            for x in range(tiles_w):
                current_tile_img = get_tile_image(full_img, x, y)
                center_pixel = current_tile_img.getpixel((7, 7))

                is_solid = 1

                # Test Couleur
                for bg_col in BACKGROUND_COLORS:
                    if (abs(center_pixel[0] - bg_col[0]) <= COLOR_TOLERANCE and
                        abs(center_pixel[1] - bg_col[1]) <= COLOR_TOLERANCE and
                            abs(center_pixel[2] - bg_col[2]) <= COLOR_TOLERANCE):
                        is_solid = 0
                        break

                # Test Modèle (Si vide, on vérifie si c'est un tuyau)
                if is_solid == 0:
                    for tmpl in templates:
                        if compare_images(current_tile_img, tmpl) < MATCH_THRESHOLD:
                            is_solid = 1
                            break

                collision_data.append(is_solid)

        # --- ETAPE 3 : Écriture ---
        with open(OUTPUT_FILENAME, "w") as f:
            f.write(f"#ifndef LEVEL_COLLISION_H\n#define LEVEL_COLLISION_H\n\n")
            f.write(f"const int MAP_WIDTH = {tiles_w};\n")
            f.write(f"const int MAP_HEIGHT = {tiles_h};\n")
            f.write(f"const int TILE_SIZE = {TILE_SIZE};\n\n")
            f.write(f"const unsigned char {ARRAY_NAME}[] = {{\n")
            for i, val in enumerate(collision_data):
                if i > 0 and i % tiles_w == 0:
                    f.write("\n\t")
                elif i == 0:
                    f.write("\t")
                f.write(str(val))
                if i < len(collision_data) - 1:
                    f.write(", ")
            f.write("\n};\n\n#endif")

        print("Fichier généré.")

        if SHOW_DEBUG:
            show_debug_view(full_img, collision_data, tiles_w, tiles_h)

    except Exception as e:
        print(f"Erreur : {e}")


if __name__ == "__main__":
    generate_collision_script()

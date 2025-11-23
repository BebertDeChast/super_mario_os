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
SHOW_COORDS = False # Mets sur True pour trouver les coordonnées des pièces/?
FONT_SIZE = 5
GRID_COLOR = 'white'
GRID_ALPHA = 0.3

# --- CONFIGURATION DE DETECTION ---
BACKGROUND_COLORS = [
    (92, 148, 252), (0, 0, 0), (255, 255, 255),
    (0, 168, 0), (128, 208, 16), (248, 56, 0)
]
COLOR_TOLERANCE = 15
MATCH_THRESHOLD = 50

# --- COORDONNÉES DES TEMPLATES (MODELES) ---
# Format: (X, Y) en tuiles. 
# IMPORTANT : J'ai mis des valeurs probables pour Mario Bros, 
# mais tu dois utiliser SHOW_COORDS=True pour vérifier et corriger si besoin !

# 1. Objets SOLIDES complexes (Tuyaux, briques sombres...) -> ID 1
SOLID_TEMPLATES_COORDS = [
    (28, 11), (29, 11), (28, 12), (29, 12),  # tuyau
    (48, 17),  # brique sombre
    (63, 17), (61, 26), (61, 27), (62, 26), (62, 27), (63, 26), (63, 27),  # tuyau
    (20, 9),  # brique marron
]

# 2. PIÈCES -> ID 2
COIN_TEMPLATES_COORDS = [
    # Trouve une pièce isolée dans ton niveau pour l'ajouter ici
    # Exemple (à vérifier avec ta grille) :
    (53, 20),
]

# 3. BLOCS "?" (Mystère) -> ID 3
MYSTERY_TEMPLATES_COORDS = [
    # Trouve un bloc ? non utilisé
    # Exemple (Le premier bloc ? du niveau) :
    (16, 9) 
]


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


def match_template_list(current_img, template_list):
    """Vérifie si l'image actuelle correspond à l'un des templates de la liste"""
    for tmpl in template_list:
        if compare_images(current_img, tmpl) < MATCH_THRESHOLD:
            return True
    return False


def show_debug_view(img, collision_data, tiles_w, tiles_h):
    print("Génération de la vue Debug...")
    fig, ax = plt.subplots(figsize=(20, 6))
    
    # Correction de l'alignement
    ax.imshow(img, extent=[0, img.width, img.height, 0])

    print(f"Dessin des overlays...")

    for y in range(tiles_h):
        for x in range(tiles_w):
            # 1. Grille
            ax.add_patch(patches.Rectangle(
                (x * TILE_SIZE, y * TILE_SIZE), TILE_SIZE, TILE_SIZE,
                linewidth=0.5, edgecolor=GRID_COLOR, facecolor='none', alpha=GRID_ALPHA
            ))

            # 2. Coordonnées (Si activé)
            if SHOW_COORDS:
                ax.text(
                    x * TILE_SIZE + 8, y * TILE_SIZE + 8,
                    f"{x}\n{y}",
                    color='cyan', fontsize=FONT_SIZE,
                    ha='center', va='center', fontweight='bold'
                )

            # 3. Coloriage selon le type (ID)
            index = y * tiles_w + x
            tile_val = collision_data[index]
            
            color = None
            if tile_val == 1:
                color = 'red'       # Solide standard
            elif tile_val == 2:
                color = 'yellow'    # Pièce
            elif tile_val == 3:
                color = 'magenta'   # Bloc ?

            if color:
                ax.add_patch(patches.Rectangle(
                    (x * TILE_SIZE, y * TILE_SIZE), TILE_SIZE, TILE_SIZE,
                    linewidth=0, facecolor=color, alpha=0.4
                ))

    # Légende pour s'y retrouver
    legend_elements = [
        patches.Patch(facecolor='red', alpha=0.4, label='Sol (1)'),
        patches.Patch(facecolor='yellow', alpha=0.4, label='Pièce (2)'),
        patches.Patch(facecolor='magenta', alpha=0.4, label='Bloc ? (3)')
    ]
    ax.legend(handles=legend_elements, loc='upper right')

    plt.title("Carte des collisions - Vue Debug")
    plt.tight_layout()
    plt.show()


def generate_collision_script():
    try:
        full_img = Image.open(INPUT_IMAGE).convert('RGB')
        tiles_w = full_img.width // TILE_SIZE
        tiles_h = full_img.height // TILE_SIZE

        # --- ETAPE 1 : Apprentissage des Templates ---
        solid_templates = []
        coin_templates = []
        mystery_templates = []

        print("Apprentissage des modèles...")
        # Fonction helper pour charger une liste
        def load_templates(coords_list, target_list):
            for tx, ty in coords_list:
                if tx < tiles_w and ty < tiles_h:
                    target_list.append(get_tile_image(full_img, tx, ty))
        
        load_templates(SOLID_TEMPLATES_COORDS, solid_templates)
        load_templates(COIN_TEMPLATES_COORDS, coin_templates)
        load_templates(MYSTERY_TEMPLATES_COORDS, mystery_templates)

        collision_data = []

        # --- ETAPE 2 : Analyse ---
        print("Analyse de la carte...")
        for y in range(tiles_h):
            for x in range(tiles_w):
                current_tile_img = get_tile_image(full_img, x, y)
                center_pixel = current_tile_img.getpixel((7, 7))

                # --- LOGIQUE DE DETECTION HIÉRARCHIQUE ---
                
                # 1. Par défaut, on suppose que c'est solide (Briques, sols...)
                tile_val = 1 

                # 2. Si c'est une couleur de fond, ça devient 0 (Air)
                for bg_col in BACKGROUND_COLORS:
                    if (abs(center_pixel[0] - bg_col[0]) <= COLOR_TOLERANCE and
                        abs(center_pixel[1] - bg_col[1]) <= COLOR_TOLERANCE and
                        abs(center_pixel[2] - bg_col[2]) <= COLOR_TOLERANCE):
                        tile_val = 0
                        break
                
                # 3. Corrections et Identifications Spécifiques (Pattern Matching)
                # Ces tests écrasent la décision prise à l'étape 2 (Couleur)
                
                # A. Est-ce un tuyau/brique sombre mal détecté ? -> On remet à 1
                if tile_val == 0: 
                    if match_template_list(current_tile_img, solid_templates):
                        tile_val = 1
                
                # B. Est-ce un Bloc Mystère ? -> 3 (Prioritaire sur Solide et Air)
                if match_template_list(current_tile_img, mystery_templates):
                    tile_val = 3
                
                # C. Est-ce une Pièce ? -> 2 (Prioritaire sur tout)
                # Note : Une pièce a souvent du fond bleu, donc étape 2 a dit "0".
                # Ici on corrige : "C'est du fond bleu mais ça ressemble à une pièce -> 2"
                elif match_template_list(current_tile_img, coin_templates):
                    tile_val = 2

                collision_data.append(tile_val)

        # --- ETAPE 3 : Écriture ---
        with open(OUTPUT_FILENAME, "w") as f:
            f.write(f"#ifndef LEVEL_COLLISION_H\n#define LEVEL_COLLISION_H\n\n")
            f.write(f"const int MAP_WIDTH = {tiles_w};\n")
            f.write(f"const int MAP_HEIGHT = {tiles_h};\n")
            f.write(f"const int TILE_SIZE = {TILE_SIZE};\n\n")
            f.write(f"// 0=Air, 1=Sol, 2=Piece, 3=Bloc ?\n")
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
        import traceback
        traceback.print_exc()


if __name__ == "__main__":
    generate_collision_script()
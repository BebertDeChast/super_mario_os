from pathlib import Path
from PIL import Image, ImageChops
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import math
import time

# ==========================================
# --- CONFIGURATION UTILISATEUR ---
# ==========================================

BASE_DIR = Path(__file__).resolve().parent.parent.parent

LEVEL_IMAGE_PATH = BASE_DIR / "sprites" / "NES - Super Mario Bros. - Stages - World 1-1.png"
TILESET_IMAGE_PATH = BASE_DIR / "sprites" / "NES - Super Mario Bros. - Miscellaneous - Tileset.png"

OUTPUT_DIR = BASE_DIR / "Applications" / "Level"
OUTPUT_FILENAME = OUTPUT_DIR / "LevelCollision.h"

ARRAY_NAME = "collision_map"
TILE_SIZE = 16 

# --- MODE VERIFICATION ---
# Mettre à True pour afficher seulement les assets découpés (pour vérifier les coordonnés)
# Mettre à False pour lancer l'analyse complète du niveau

CHECK_ASSETS_MODE = False

# --- DEBUG SETTINGS ---
SHOW_DEBUG = False
SHOW_COORDS = False 
FONT_SIZE = 5
GRID_COLOR = 'white'
GRID_ALPHA = 0.3

# --- REGLAGES ANALYSE ---
MATCH_THRESHOLD = 100   # Tolérance de ressemblance (0 = identique)
COLOR_TOLERANCE = 15   # Tolérance pour les couleurs de fond

# --- DEFINITION DES TYPES DE COLLISION ET INTERACTIONS ---
PASSTHROUGH = 0
NOT_PASSTHROUGH = 1
COIN = 2
QUESTION_BOX = 3

# --- DEFINITION DES ASSETS A RECONNAITRE ---
# Format : "Nom": { "x": pixel_x, "y": pixel_y, "type": ID_COLLISION }
# Types : 1 = Solide, 2 = Pièce, 3 = Bloc ? (Mystère)
# NOTE : Tu dois ouvrir ton image Tileset dans Paint/Gimp pour trouver les X,Y du coin haut-gauche de chaque sprite.

ASSETS_CONFIG = {
    # --- OBJETS SPECIAUX (Prioritaires) ---
    "Coin":         {"x": 298, "y": 95,  "type": COIN}, # Pièce (Palette 3)
    "QuestionBox":  {"x": 298, "y": 78,  "type": QUESTION_BOX}, # Bloc ? (Palette 3)
    "Coin_underground_FCEUX": {"x": 428, "y": 524, "type": COIN}, # Pièce (Sous-sol Palette 0)
    
    # --- SOLIDES (Briques, Sols, Tuyaux) ---
    "Ground":       {"x": 0,   "y": 16,  "type": NOT_PASSTHROUGH}, # Sol marron (Palette 1)
    "Brick_with_way":        {"x": 17,  "y": 16,  "type": NOT_PASSTHROUGH}, # Brique marron (Palette 1)
    "HardBlock":    {"x": 0,   "y": 33,  "type": NOT_PASSTHROUGH}, # Bloc fer (Escalier fin niveau)
    "Brick_underground_with_way": {"x": 164, "y": 16, "type": NOT_PASSTHROUGH}, # Brique Sous-sol (Palette 0)
    "Brick_underground_no_way": {"x": 181, "y": 16, "type": NOT_PASSTHROUGH}, # Brique Sous-sol (Palette 0)
    
    # Tuyaux (Palette 0 - Pipes)
    "Head_Pipe_TL":      {"x": 119,   "y": 196, "type": NOT_PASSTHROUGH}, # Tuyau Haut-Gauche
    "Head_Pipe_TR":      {"x": 136,  "y": 196, "type": NOT_PASSTHROUGH}, # Tuyau Haut-Droit
    "Pipe_BL":      {"x": 119,   "y": 213, "type": NOT_PASSTHROUGH}, # Tuyau Bas-Gauche
    "Pipe_BR":      {"x": 136,  "y": 213, "type": NOT_PASSTHROUGH}, # Tuyau Bas-Droit
    "Head_horizontal_Pipe_TL": {"x": 85, "y": 230, "type": NOT_PASSTHROUGH}, # Tuyau Horizontal Haut-Gauche
    "Head_horizontal_Pipe_BL": {"x": 85, "y": 247, "type": NOT_PASSTHROUGH}, # Tuyau Horizontal Bas-Gauche
    "horizontal_Pipe_T": {"x": 102, "y": 230, "type": NOT_PASSTHROUGH}, # Tuyau Horizontal Haut
    "horizontal_Pipe_B": {"x": 102, "y": 247, "type": NOT_PASSTHROUGH}, # Tuyau Horizontal Bas
    "coude_pipe_tl": {"x": 119, "y": 230, "type": NOT_PASSTHROUGH}, # Coude Tuyau Haut-Gauche
    "coude_pipe_bl": {"x": 119, "y": 247, "type": NOT_PASSTHROUGH}, # Coude Tuyau Bas-Gauche

    "Underground_Head_Pipe_TL":      {"x": 283,   "y": 196, "type": NOT_PASSTHROUGH}, # Tuyau Haut-Gauche
    "Underground_Head_Pipe_TR":      {"x": 300,  "y": 196, "type": NOT_PASSTHROUGH},  # Tuyau Haut-Droit
    "Underground_Pipe_BL":           {"x": 283,   "y": 213, "type": NOT_PASSTHROUGH}, # Tuyau Bas-Gauche
    "Underground_Pipe_BR":           {"x": 300,  "y": 213, "type": NOT_PASSTHROUGH},  # Tuyau Bas-Droit
    "Underground_Head_horizontal_Pipe_TL": {"x": 249, "y": 230, "type": NOT_PASSTHROUGH},  # Tuyau Horizontal Haut-Gauche
    "Underground_Head_horizontal_Pipe_BL": {"x": 249, "y": 247, "type": NOT_PASSTHROUGH},  # Tuyau Horizontal Bas-Gauche
    "Underground_horizontal_Pipe_T": {"x": 266, "y": 230, "type": NOT_PASSTHROUGH},  # Tuyau Horizontal Haut
    "Underground_horizontal_Pipe_B": {"x": 266, "y": 247, "type": NOT_PASSTHROUGH},  # Tuyau Horizontal Bas
    "Underground_coude_pipe_tl": {"x": 283, "y": 230, "type": NOT_PASSTHROUGH},   # Coude Tuyau Haut-Gauche
    "Underground_coude_pipe_bl": {"x": 283, "y": 247, "type": NOT_PASSTHROUGH},   # Coude Tuyau Bas-Gauche

    # Collines
    "Hill_1":       {"x": 0,  "y": 247,   "type": PASSTHROUGH}, 
    "Hill_2":       {"x": 17, "y": 247,   "type": PASSTHROUGH},
    "Hill_3":       {"x": 34, "y": 247,   "type": PASSTHROUGH},
    "Hill_4":       {"x": 51, "y": 247,   "type": PASSTHROUGH},
    "Hill_5":       {"x": 68, "y": 247,   "type": PASSTHROUGH},
}

# --- COULEURS DE FOND (Ignorées) ---
BACKGROUND_COLORS = [
    (92, 148, 252), (0, 0, 0), (255, 255, 255),
    (0, 168, 0), (128, 208, 16), (248, 56, 0)
]

# ==========================================
# --- MOTEUR ---
# ==========================================

def get_sub_image(img, x, y, width, height):
    return img.crop((x, y, x + width, y + height))

def compare_images(img1, img2):
    """Retourne la différence (RMS) entre deux images. 0 = Identique."""
    if img1.size != img2.size:
        return 9999
    diff = ImageChops.difference(img1, img2)
    if not diff.getbbox():
        return 0
    h = diff.histogram()
    sq = (value * ((idx % 256) ** 2) for idx, value in enumerate(h))
    sum_of_squares = sum(sq)
    return math.sqrt(sum_of_squares / float(img1.size[0] * img1.size[1]))

def check_assets_integrity(tileset_img):
    """Affiche une planche contact des assets découpés pour vérification visuelle"""
    print("--- MODE VÉRIFICATION DES ASSETS ---")
    print("Vérifiez que les images ci-dessous sont bien centrées.")
    
    count = len(ASSETS_CONFIG)
    cols = 5
    rows = math.ceil(count / cols)
    
    fig, axes = plt.subplots(rows, cols, figsize=(10, rows * 2))
    axes = axes.flatten()
    
    for i, (name, data) in enumerate(ASSETS_CONFIG.items()):
        ax = axes[i]
        # Découpe
        sprite = get_sub_image(tileset_img, data['x'], data['y'], TILE_SIZE, TILE_SIZE)
        
        # Affichage
        ax.imshow(sprite)
        ax.set_title(f"{name}\n({data['x']},{data['y']}) -> T{data['type']}", fontsize=8)
        ax.axis('off')
        
        # Petit contour rouge pour vérifier les bords
        rect = patches.Rectangle((0, 0), 15, 15, linewidth=1, edgecolor='red', facecolor='none')
        ax.add_patch(rect)

    # Cacher les cases vides
    for j in range(i + 1, len(axes)):
        axes[j].axis('off')

    plt.tight_layout()
    plt.show()

def show_debug_map(level_img, collision_data, tiles_w, tiles_h):
    print("Génération de la vue Debug...")
    fig, ax = plt.subplots(figsize=(20, 6))
    ax.imshow(level_img, extent=[0, level_img.width, level_img.height, 0])

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
            idx = y * tiles_w + x
            val = collision_data[idx]
            
            color = None
            if val == NOT_PASSTHROUGH:
                color = 'red'
            elif val == COIN:
                color = 'yellow'
            elif val == QUESTION_BOX:
                color = 'magenta'

            if color:
                ax.add_patch(patches.Rectangle(
                    (x * TILE_SIZE, y * TILE_SIZE), TILE_SIZE, TILE_SIZE,
                    linewidth=0, facecolor=color, alpha=0.4
                ))
    
    legend_elements = [
        patches.Patch(facecolor='red', alpha=0.4, label='Sol (1)'),
        patches.Patch(facecolor='yellow', alpha=0.4, label='Pièce (2)'),
        patches.Patch(facecolor='magenta', alpha=0.4, label='Bloc ? (3)')
    ]
    ax.legend(handles=legend_elements, loc='lower right')
    plt.title("Résultat de l'analyse de collision")
    plt.show()

def main():
    try:
        # S'assurer que le dossier de sortie existe (tous OS)
        OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

        # 1. Chargement des images
        print("Chargement des images...")
        tileset_img = Image.open(TILESET_IMAGE_PATH).convert('RGB')
        level_img = Image.open(LEVEL_IMAGE_PATH).convert('RGB')

        # 2. Mode Vérification (Arrêt ici si activé)
        if CHECK_ASSETS_MODE:
            check_assets_integrity(tileset_img)
            return

        # 3. Préparation des templates en mémoire
        print("Extraction des templates depuis le tileset...")
        templates = []
        for name, data in ASSETS_CONFIG.items():
            img_asset = get_sub_image(tileset_img, data['x'], data['y'], TILE_SIZE, TILE_SIZE)
            templates.append({
                'img': img_asset,
                'type': data['type'],
                'name': name
            })

        # 4. Analyse du niveau
        tiles_w = level_img.width // TILE_SIZE
        tiles_h = level_img.height // TILE_SIZE
        collision_data = []
        t1 = time.time()

        print(f"Analyse du niveau ({tiles_w} x {tiles_h} tuiles)...")
        
        for y in range(tiles_h):
            for x in range(tiles_w):
                print(f"Analyse tuile ({x}, {y})...", end="\r")
                current_tile = get_sub_image(level_img, x*TILE_SIZE, y*TILE_SIZE, TILE_SIZE, TILE_SIZE)
                center_pixel = current_tile.getpixel((7, 7))
                
                final_type = 1  # Par défaut solide
                
                # A. Test Couleur de Fond
                is_bg = False
                for bg in BACKGROUND_COLORS:
                    if (abs(center_pixel[0]-bg[0]) <= COLOR_TOLERANCE and
                        abs(center_pixel[1]-bg[1]) <= COLOR_TOLERANCE and
                        abs(center_pixel[2]-bg[2]) <= COLOR_TOLERANCE):
                        is_bg = True
                        break
                
                if is_bg:
                    final_type = 0

                # B. Pattern Matching (Assets)
                # On teste chaque asset. L'ordre de priorité : 2 (Coin) > 3 (Quest) > 1 (Sol)
                # Pour gérer ça, on cherche le meilleur match
                best_match_score = MATCH_THRESHOLD
                matched_type = -1

                for tmpl in templates:
                    score = compare_images(current_tile, tmpl['img'])
                    if score == 0:
                        # Match parfait, on peut sortir de la boucle
                        matched_type = tmpl['type']
                        break
                    if score < best_match_score:
                        best_match_score = score
                        matched_type = tmpl['type']
                
                # Si on a trouvé un match dans les assets, il écrase la logique couleur
                if matched_type != -1:
                    final_type = matched_type
                
                collision_data.append(final_type)

        print(f"Analyse terminée en {time.time() - t1:.2f} secondes.")

        # 5. Export (chemin portable)
        print(f"Génération de {OUTPUT_FILENAME}...")
        with OUTPUT_FILENAME.open("w", encoding="utf-8") as f:
            f.write(f"#ifndef LEVEL_COLLISION_H\n#define LEVEL_COLLISION_H\n\n")
            f.write(f"const int MAP_WIDTH = {tiles_w};\n")
            f.write(f"const int MAP_HEIGHT = {tiles_h};\n")
            f.write(f"const unsigned char {ARRAY_NAME}[] = {{\n")
            for i, val in enumerate(collision_data):
                if i > 0 and i % tiles_w == 0:
                    f.write("\n\t")
                elif i == 0:
                    f.write("\t")
                f.write(str(val))
                if i < len(collision_data)-1:
                    f.write(", ")
            f.write("\n};\n\n#endif")

        if SHOW_DEBUG:
            show_debug_map(level_img, collision_data, tiles_w, tiles_h)

    except FileNotFoundError as e:
        print(f"ERREUR FICHIER : {e}")
    except Exception as e:
        print(f"ERREUR : {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    main()

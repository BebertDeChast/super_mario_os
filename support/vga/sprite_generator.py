from pathlib import Path
from PIL import Image
import sys
import math

# Configuration
BASE_DIR = Path(__file__).resolve().parent.parent.parent

INPUT_IMAGE = BASE_DIR / "sprites" / "NES - Super Mario Bros. - Stages - World 1-1.png"
INPUT_PALETTE = BASE_DIR / "support" / "vga" / "atari-8-bit-family-gtia.pal"
OUTPUT_HEADER = BASE_DIR / "Applications" / "Level" / "Level_display_data.h"
VAR_NAME = "level_sprite_indices"

# ---------------------------------------------------------
# 1. Logique de lecture de palette (adapté de palette.py)
# ---------------------------------------------------------
def parse_jasc(filename):
    """Lit un fichier palette JASC-PAL et retourne une liste de tuples (R, G, B)."""
    try:
        with open(filename, 'r') as f:
            lines = f.readlines()
        
        # Vérification basique de l'en-tête
        if not lines[0].startswith("JASC-PAL"):
            print(f"Attention: {filename} ne semble pas être un fichier JASC-PAL standard.")

        # Les données commencent après les 3 premières lignes (Header, Version, Count)
        # On lit jusqu'à 256 couleurs
        raw_data = lines[3:3+256]
        palette = []
        for l in raw_data:
            parts = list(map(int, l.split()))
            # On s'assure d'avoir R, G, B
            if len(parts) >= 3:
                palette.append((parts[0], parts[1], parts[2]))
            else:
                # Fallback noir si ligne malformée
                palette.append((0, 0, 0))
        return palette
    except Exception as e:
        print(f"Erreur lors de la lecture de la palette : {e}")
        sys.exit(1)

# ---------------------------------------------------------
# 2. Logique de comparaison de couleurs
# ---------------------------------------------------------
def get_nearest_color_index(pixel, palette, cache):
    """
    Trouve l'index de la couleur la plus proche dans la palette.
    Utilise un cache pour éviter de recalculer la distance pour les mêmes pixels.
    """
    # pixel est un tuple (R, G, B)
    if pixel in cache:
        return cache[pixel]

    pr, pg, pb = pixel
    min_dist = float('inf')
    best_index = 0

    for i, (cr, cg, cb) in enumerate(palette):
        # Distance Euclidienne au carré (pas besoin de racine carrée pour comparer)
        dist = (pr - cr)**2 + (pg - cg)**2 + (pb - cb)**2
        if dist < min_dist:
            min_dist = dist
            best_index = i
            # Optimisation : si correspondance exacte, on arrête
            if dist == 0:
                break
    
    cache[pixel] = best_index
    return best_index

# ---------------------------------------------------------
# 3. Générateur principal
# ---------------------------------------------------------
def generate_indexed_sprite():
    # A. Chargement de la palette
    print(f"Chargement de la palette : {INPUT_PALETTE}")
    atari_palette = parse_jasc(INPUT_PALETTE)
    print(f"Palette chargée : {len(atari_palette)} couleurs.")

    # B. Chargement de l'image
    try:
        img = Image.open(INPUT_IMAGE).convert('RGB')
        width, height = img.size
        pixels = img.load()
        print(f"Traitement de l'image : {width}x{height} pixels.")
    except FileNotFoundError:
        print(f"Erreur : L'image '{INPUT_IMAGE}' est introuvable.")
        sys.exit(1)

    # C. Conversion et Écriture
    color_cache = {} # Pour accélérer le traitement
    
    print("Conversion des pixels vers les indices de palette (quantification)...")
    
    with open(OUTPUT_HEADER, "w") as f:
        # En-tête du fichier C++
        f.write(f"#ifndef LEVEL_DATA_INDEXED_H\n#define LEVEL_DATA_INDEXED_H\n\n")
        f.write(f"// Généré à partir de : {INPUT_IMAGE}\n")
        f.write(f"// Palette utilisée   : {INPUT_PALETTE}\n\n")
        f.write(f"const int LEVEL_WIDTH = {width};\n")
        f.write(f"const int LEVEL_HEIGHT = {height};\n\n")
        
        # Tableau 1D d'indices (unsigned char)
        f.write(f"const unsigned char {VAR_NAME}[] = {{\n")

        for y in range(height):
            line_indices = []
            for x in range(width):
                pixel = pixels[x, y] 
                index = get_nearest_color_index(pixel, atari_palette, color_cache)
                line_indices.append(f"{index}")
            
            # Écrire la ligne
            f.write("    " + ", ".join(line_indices))
            
            if y < height - 1:
                f.write(",\n")
            else:
                f.write("\n")
        
        f.write("};\n\n#endif")
        
    print(f"Terminé ! Fichier généré : {OUTPUT_HEADER}")

if __name__ == "__main__":
    generate_indexed_sprite()
from PIL import Image
import sys

# Configuration
INPUT_IMAGE = "sprites/NES - Super Mario Bros. - Playable Characters - Mario & Luigi.png"
INPUT_PALETTE = "support/vga/atari-8-bit-family-gtia.pal"
OUTPUT_HEADER = "sprites/MarioSprites.h"

# Coordonnées du Mario dans l'image (A AJUSTER selon ton image)
# Astuce : Ouvre l'image dans Paint pour trouver le X,Y du coin haut-gauche du Mario
WIDTH = 16
HEIGHT = 16
MARIO_X = 20
MARIO_Y = 8

def parse_jasc(filename):
    try:
        with open(filename, 'r') as f:
            lines = f.readlines()
        raw_data = lines[3:3+256]
        palette = []
        for l in raw_data:
            parts = list(map(int, l.split()))
            if len(parts) >= 3:
                palette.append((parts[0], parts[1], parts[2]))
            else:
                palette.append((0, 0, 0))
        return palette
    except Exception as e:
        print(f"Erreur palette: {e}")
        sys.exit(1)

def get_nearest_color_index(pixel, palette):
    pr, pg, pb = pixel
    min_dist = float('inf')
    best_index = 0
    for i, (cr, cg, cb) in enumerate(palette):
        dist = (pr - cr)**2 + (pg - cg)**2 + (pb - cb)**2
        if dist < min_dist:
            min_dist = dist
            best_index = i
            if dist == 0: break
    return best_index

def generate_mario():
    try:
        print(f"Chargement palette: {INPUT_PALETTE}")
        palette = parse_jasc(INPUT_PALETTE)

        img = Image.open(INPUT_IMAGE).convert('RGB')
        # Découpage
        sprite = img.crop((MARIO_X, MARIO_Y, MARIO_X + WIDTH, MARIO_Y + HEIGHT))
        
        pixels = list(sprite.getdata())
        
        # Mario reversed (flipped on Y axis)
        sprite_reversed = sprite.transpose(Image.FLIP_LEFT_RIGHT)
        pixels_reversed = list(sprite_reversed.getdata())
        
        # On suppose que le pixel en haut à gauche (0,0) est la couleur de fond (transparence)
        bg_color = pixels[0]
        
        with open(OUTPUT_HEADER, "w") as f:
            f.write("#ifndef MARIO_SPRITES_H\n#define MARIO_SPRITES_H\n\n")
            f.write(f"#define MARIO_SPRITE_WIDTH {WIDTH}\n")
            f.write(f"#define MARIO_SPRITE_HEIGHT {HEIGHT}\n\n")
            f.write("// Mario facing right\n")
            f.write(f"static unsigned char marioSpriteData[MARIO_SPRITE_WIDTH * MARIO_SPRITE_HEIGHT] = {{\n")
            
            cache = {}
            
            def write_pixels(pxls):
                for i, p in enumerate(pxls):
                    if p == bg_color:
                        val = 255 # Index 255 = Transparent
                    elif p in cache:
                        val = cache[p]
                    else:
                        val = get_nearest_color_index(p, palette)
                        # Si la couleur la plus proche est 0 (noir/transparent) mais que ce n'est pas le fond,
                        # on force l'index 1 (gris très sombre) pour éviter les trous dans le sprite.
                        if val == 0:
                            val = 0
                        cache[p] = val

                    f.write(f"0x{val:02X}, ")
                    if (i + 1) % 16 == 0:
                        f.write("\n")
            
            write_pixels(pixels)
            
            f.write("};\n\n")
            
            f.write("// Mario facing left\n")
            f.write(f"static unsigned char marioSpriteDataReversed[MARIO_SPRITE_WIDTH * MARIO_SPRITE_HEIGHT] = {{\n")
            write_pixels(pixels_reversed)
            f.write("};\n\n#endif")
            
        print(f"Sprite généré : {OUTPUT_HEADER}")
        
    except Exception as e:
        print(f"Erreur : {e}")

if __name__ == "__main__":
    generate_mario()

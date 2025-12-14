import sys
from pathlib import Path
from typing import List, Tuple, Dict, Optional
from PIL import Image

# ---------------------------------------------------------
# Configuration
# ---------------------------------------------------------
BASE_DIR = Path(__file__).resolve().parents[2]

VAR_NAME = "TITLE_SCREEN"
INPUT_IMAGE = BASE_DIR / "sprites" / "NES - Super Mario Bros.Title Screen.png"
INPUT_PALETTE = BASE_DIR / "support" / "vga" / "atari-8-bit-family-gtia.pal"
OUTPUT_HEADER = BASE_DIR / "sprites" / "titleScreenSprite.h"
TRANSPARENT_IDX = 138  # The index in the source palette to treat as transparent
OUTPUT_TRANSPARENT_VAL = 255  # The value to write to the file for transparency

# Type alias for RGB Color
Color = Tuple[int, int, int]
Palette = List[Color]


# ---------------------------------------------------------
# 1. Palette Loading
# ---------------------------------------------------------
def load_jasc_palette(filepath: Path) -> Palette:
    """
    Parses a JASC-PAL file and returns a list of RGB tuples.
    """
    if not filepath.exists():
        raise FileNotFoundError(f"Palette file not found: {filepath}")

    try:
        with open(filepath, 'r') as f:
            lines = [line.strip() for line in f.readlines() if line.strip()]

        if not lines[0].startswith("JASC-PAL"):
            print(f"Warning: {filepath} header is not JASC-PAL.")

        # JASC-PAL format: Header, Version, Count, then data.
        # Data usually starts at line index 3.
        raw_data = lines[3:3 + 256]
        palette: Palette = []

        for line in raw_data:
            parts = list(map(int, line.split()))
            if len(parts) >= 3:
                palette.append((parts[0], parts[1], parts[2]))
            else:
                palette.append((0, 0, 0))  # Fallback to black

        return palette

    except Exception as e:
        raise RuntimeError(f"Failed to parse palette: {e}")


# ---------------------------------------------------------
# 2. Color Matching Logic
# ---------------------------------------------------------
def get_nearest_color_index(pixel: Color, palette: Palette, cache: Dict[Color, int]) -> int:
    """
    Finds the index of the nearest color in the palette using Euclidean distance.
    Uses a cache to speed up lookups for recurring pixel colors.
    """
    if pixel in cache:
        return cache[pixel]

    pr, pg, pb = pixel
    min_dist = float('inf')
    best_index = 0

    for i, (cr, cg, cb) in enumerate(palette):
        # Squared Euclidean distance (sqrt is unnecessary for comparison)
        dist = (pr - cr) ** 2 + (pg - cg) ** 2 + (pb - cb) ** 2

        if dist < min_dist:
            min_dist = dist
            best_index = i
            if dist == 0:  # Exact match found
                break

    cache[pixel] = best_index
    return best_index


# ---------------------------------------------------------
# 3. Image Processing
# ---------------------------------------------------------
def map_pixels_to_indices(img: Image.Image, palette: Palette) -> List[str]:
    """
    Converts image pixels to palette indices. Returns a list of strings
    representing the indices for easier file writing.
    """
    width, height = img.size
    pixels = img.load()
    color_cache: Dict[Color, int] = {}

    indexed_data = []

    # Pre-fetch transparency target to avoid index lookups if exact match
    target_transparent_color = None
    if 0 <= TRANSPARENT_IDX < len(palette):
        target_transparent_color = palette[TRANSPARENT_IDX]

    print("Quantizing pixels...")

    for y in range(height):
        row_indices = []
        for x in range(width):
            pixel = pixels[x, y]

            # Determine index
            index = get_nearest_color_index(pixel, palette, color_cache)

            # Handle Transparency logic
            # If the closest color is the transparent key, replace with output transparency value
            if index == TRANSPARENT_IDX:
                index = OUTPUT_TRANSPARENT_VAL

            # (Optional) If we want exact RGB matching for transparency override:
            if target_transparent_color and pixel == target_transparent_color:
                index = OUTPUT_TRANSPARENT_VAL

            row_indices.append(str(index))

        indexed_data.append(", ".join(row_indices))

    return indexed_data


# ---------------------------------------------------------
# 4. C++ Header Generation
# ---------------------------------------------------------
def write_cpp_header(filepath: Path, var_name: str, width: int, height: int, data_rows: List[str]):
    """
    Writes the indexed data to a C++ header file.
    """
    print(f"Writing output to: {filepath}")

    with open(filepath, "w", encoding="utf-8") as f:
        f.write(f"#ifndef {var_name}_SPRITE_H\n#define {var_name}_SPRITE_H\n\n")
        f.write(f"// Generated from: {INPUT_IMAGE.name}\n")
        f.write(f"// Palette used  : {INPUT_PALETTE.name}\n\n")

        f.write(f"const int {var_name}_WIDTH = {width};\n")
        f.write(f"const int {var_name}_HEIGHT = {height};\n\n")

        f.write(f"const unsigned char {var_name.lower()}_sprite[] = {{\n")

        for i, row_str in enumerate(data_rows):
            line_end = ",\n" if i < len(data_rows) - 1 else "\n"
            f.write(f"    {row_str}{line_end}")

        f.write("};\n\n#endif")


# ---------------------------------------------------------
# Main Execution
# ---------------------------------------------------------
def main():
    print(f"--- Sprite Generator: {VAR_NAME} ---")

    # 1. Load Palette
    try:
        print(f"Loading palette: {INPUT_PALETTE}")
        atari_palette = load_jasc_palette(INPUT_PALETTE)
        print(f"Palette loaded: {len(atari_palette)} colors.")
    except Exception as e:
        print(f"Error loading palette: {e}")
        sys.exit(1)

    # 2. Load Image
    try:
        if not INPUT_IMAGE.exists():
            raise FileNotFoundError(
                f"Image not found: {INPUT_IMAGE}")

        img = Image.open(INPUT_IMAGE).convert('RGB')
        print(f"Image loaded: {img.size[0]}x{img.size[1]} pixels.")
    except Exception as e:
        print(f"Error loading image: {e}")
        sys.exit(1)

    # 3. Process Data
    try:
        indexed_rows = map_pixels_to_indices(img, atari_palette)
    except Exception as e:
        print(f"Error processing image data: {e}")
        sys.exit(1)

    # 4. Write Output
    try:
        write_cpp_header(
            OUTPUT_HEADER,
            VAR_NAME,
            img.size[0],
            img.size[1],
            indexed_rows
        )
        print("Success! Generation complete.")
    except Exception as e:
        print(f"Error writing output file: {e}")
        sys.exit(1)


if __name__ == "__main__":
    main()

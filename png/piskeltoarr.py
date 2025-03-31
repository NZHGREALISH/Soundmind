from PIL import Image

# 📂 INPUT FILE: Change this to your PNG file path
input_png = "Base_bg_level4.png"
output_c_file = "stage4.h"

# 📏 Expected image size (adjust if needed)
SCREEN_WIDTH = 320
SCREEN_HEIGHT = 240

# Convert RGB888 to RGB565
def rgb888_to_rgb565(r, g, b):
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

# Load image
image = Image.open(input_png).convert("RGBA")  # Convert to RGBA for transparency handling
image = image.resize((SCREEN_WIDTH, SCREEN_HEIGHT))  # Resize if necessary

# Prepare RGB565 data
rgb565_data = []

for y in range(SCREEN_HEIGHT):
    row = []
    for x in range(SCREEN_WIDTH):
        r, g, b, a = image.getpixel((x, y))  # Get RGBA values
        
        # 🛑 Handle transparency: Convert transparent pixels to black (or another color)
        if a < 128:  
            r, g, b = 0, 0, 0  # Set transparent areas to black
        
        # Convert to 16-bit RGB565
        rgb565 = rgb888_to_rgb565(r, g, b)
        row.append(f"0x{rgb565:04X}")
    
    rgb565_data.append(", ".join(row))

# Write output to a C header file
with open(output_c_file, "w") as f:
    f.write("#ifndef BACKGROUND_H\n#define BACKGROUND_H\n\n")
    f.write("#define SCREEN_WIDTH  320\n#define SCREEN_HEIGHT 240\n\n")
    f.write("uint16_t background[SCREEN_HEIGHT][SCREEN_WIDTH] = {\n")
    
    for row in rgb565_data:
        f.write("    {" + row + "},\n")
    
    f.write("};\n\n#endif\n")

print(f"✅ PNG successfully converted to {output_c_file}")

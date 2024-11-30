import os
import sys

def convert_to_header(file_path):
    print(f"Converting {file_path} to header")
    base_name = os.path.splitext(os.path.basename(file_path))[0]
    output_file = f"{base_name}.h"
    with open(file_path, "rb") as f, open(output_file, "w") as out:
        data = f.read()
        out.write(f"const unsigned char g_{base_name}_bin[] = {{\n")
        out.write(", ".join(f"0x{byte:02x}" for byte in data))
        out.write("\n};\n")
        out.write(f"const unsigned int g_{base_name}_size = {len(data)};\n")


def generate_assets():
    for root, dirs, files in os.walk("../assets"):
        for file in files:
            file_path = os.path.join(root, file)
            convert_to_header(file_path)

generate_assets()

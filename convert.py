import os

# Update this to match your exact file name if different
tflite_filename = "model_quant_int8.tflite" 
header_filename = "model_data.h"

if not os.path.exists(tflite_filename):
    print(f"Error: Could not find '{tflite_filename}'. Please check the file name and path.")
else:
    with open(tflite_filename, "rb") as f:
        bytes_data = f.read()

    with open(header_filename, "w") as f:
        f.write("#ifndef MODEL_DATA_H\n#define MODEL_DATA_H\n\n")
        f.write("// Align to 8-byte boundary for ESP32 memory access\n")
        f.write("alignas(8) const unsigned char g_model[] = {\n  ")
        
        for i, b in enumerate(bytes_data):
            f.write(f"0x{b:02x}, ")
            if (i + 1) % 12 == 0:
                f.write("\n  ")
                
        f.write("\n};\n")
        f.write(f"const unsigned int g_model_len = {len(bytes_data)};\n\n")
        f.write("#endif // MODEL_DATA_H\n")

    print(f"Success! Created '{header_filename}' ({len(bytes_data)} bytes).")
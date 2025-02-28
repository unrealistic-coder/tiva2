import os
import gzip
import sys

def gzip_file(input_path, output_path):
    try:
        with open(input_path, 'rb') as f_in:
            with gzip.open(output_path, 'wb') as f_out:
                while True:
                    chunk = f_in.read(4096)
                    if not chunk:
                        break
                    f_out.write(chunk)
    except Exception as e:
        print(f"Failed to process {input_path}: {e}")

def main(input_folder, output_folder):
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    for root, dirs, files in os.walk(input_folder):
        for file in files:
            input_path = os.path.join(root, file)
            relative_path = os.path.relpath(root, input_folder)
            target_root = os.path.join(output_folder, relative_path)
            os.makedirs(target_root, exist_ok=True)
            # Append .gz to the original filename
            new_filename = file 
            output_path = os.path.join(target_root, new_filename)
            gzip_file(input_path, output_path)
            print(f"Compressed: {input_path} -> {output_path}")

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python gzip_compressor.py <input_folder> <output_folder>")
        sys.exit(1)
    input_folder = sys.argv[1]
    output_folder = sys.argv[2]
    if not os.path.isdir(input_folder):
        print(f"Error: '{input_folder}' is not a valid directory.")
        sys.exit(1)
    main(input_folder, output_folder)
import os
import zlib


def compress_file(file_path, output_folder):
    with open(file_path, 'rb') as f:
        data = f.read()
        compressed_data = zlib.compress(data, level=9)

    compressed_file_name = os.path.basename(file_path).upper()
    compressed_file_path = os.path.join(output_folder, compressed_file_name)
    
    with open(compressed_file_path, 'wb') as f:
        f.write(compressed_data)

    print(f"Compressed: {file_path} -> {compressed_file_path}")


def compress_folder(input_folder, output_folder):
    if not os.path.exists(output_folder):
        os.makedirs(output_folder)
    
    for root, _, files in os.walk(input_folder):
        for file in files:
            file_path = os.path.join(root, file)
            compress_file(file_path, output_folder)


if __name__ == "__main__":
    input_folder = r"D:\websock_FATFS\fs_work"
    output_folder = r"D:\websock_FATFS\fs_deflate"
    
    if os.path.isdir(input_folder):
        compress_folder(input_folder, output_folder)
        print("Compression completed.")
    else:
        print("Invalid folder path.")

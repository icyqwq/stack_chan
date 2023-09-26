import os
import struct
import io
import argparse
import sys

head_offset = 0 * 4096 
file_offset = 2 * 4096
file_align_size = 4096

def crc16(data):
    PRESET = 0xFFFF
    POLYNOMIAL = 0xA001 # bit reverse of 0x8005
    crc = PRESET
    for c in data:
        crc = crc ^ c
        for j in range(8):
            if crc & 0x01:
                crc = (crc >> 1) ^ POLYNOMIAL
            else:
                crc = crc >> 1
    return crc

def get_files(path_in, offset_start):
    files_map = {}
    files = os.listdir(path_in)
    for i in files:
        file = path_in + '/' + i
        if os.path.isfile(file):
            file_actual_size = (os.stat(file).st_size + file_align_size - 1) // 4096 * 4096
            files_map[i] = {
                "offset": offset_start, 
                "size": os.stat(file).st_size, 
                "align_fill_size": file_actual_size - os.stat(file).st_size, 
                "path": file
            }
        offset_start += file_actual_size
    return files_map

# | mark_valid |   name   | data_offset | data_size |   CRC   |
# |   1 byte   |  35 byte |    8 byte   |   4 byte  |  2byte  |
def generate_title(name, offset, size, crc):
    data = struct.pack("B35s", 1, name.encode()) + struct.pack("<QI", offset, size)
    data += struct.pack("<H", crc)
    return data

def generate_bin(file_path, out_path, max_size):
    files_info = get_files(file_path, file_offset)
    head_data = b''
    data = io.BytesIO()
    data.seek(file_offset)
    offset_now = file_offset

    for file_name in sorted(files_info):
        file_info = files_info[file_name]
        if file_info["offset"] + file_info["size"] + file_info["align_fill_size"] > max_size:
            raise OSError("File too large")
        with open(file_info["path"], 'rb') as fin:
            file_data = fin.read()
            crc_value = crc16(file_data)
            data.seek(file_info["offset"])
            data.write(file_data)
            data.write(b'\xff' * file_info["align_fill_size"])
            offset_now += len(file_data) + file_info["align_fill_size"]
        head_data += generate_title(file_name, file_info["offset"], file_info["size"], crc_value)

    data.seek(0)
    data.write(head_data)
    data.write(b'\xff' * (file_offset - len(head_data)))
    data.seek(0)
    with open(out_path, 'wb') as fout:
        fout.write(data.read())

def main():
    parser = argparse.ArgumentParser(description='QMSD User File Generator')
    parser.add_argument('base_dir', help='Path to directory from which the image will be created')
    parser.add_argument('image_size', help='Size of the created image')
    parser.add_argument('output_file', help='Created image output file path')
    args = parser.parse_args()

    if not os.path.exists(args.base_dir):
        raise RuntimeError('given base directory %s does not exist' % args.base_dir)

    image_size = int(args.image_size, 0)
    try:
        generate_bin(args.base_dir, args.output_file, image_size)
        print(f"QMSD ezfile generator success -> {args.output_file} \-_-/ !!")
    except OSError as e:
        print(f"QMSD user file generator failed, {e}")
        sys.exit(1)

if __name__ == '__main__':
    main()

#!/usr/bin/env python3
import hashlib, struct, sys

MAGIC = 0x4E495A41
HEADER_SIZE = 512

def build(app_bin, out_bin, version):
    try:
        with open(app_bin, 'rb') as f:
            body = f.read()  # Read the application binary file in binary mode, without the b in rb, it would read it in text mode, which could cause issues with binary data.
    except FileNotFoundError:
        print(f"Error: File not found - {app_bin}")
        sys.exit(1)
    except IOError:
        print(f"Error: Unable to read file - {app_bin}")
        sys.exit(1)

    digest = hashlib.sha256(body).digest()  # Hash the whole file in one call. .digest() gives 32 raw bytes, .hexdigest() gives a hex string. We want the raw bytes for the header.

    """
    We need to generate a header for the image. The header is 512 bytes long and contains the following fields:
    < I I I I 32s 64s
    Symbols     Meaning
    <           Little-endian - For the Cortex-M
    I           Unsigned integer - unit32_t (4 bytes)
                    magic
                    version
                    img_len
                    reserved
    32s         32 bytes string - SHA256 digest
    64s         64 bytes string - Reserved for future use    
    """
    header = struct.pack('<IIII32s64s',
                         MAGIC, version, len(body), 0,
                         digest, b'\x00' * 64)              # b'\x00' * 64 is 64 zero bytes
    
    header += b'\x00' * (HEADER_SIZE - len(header))
    assert len(header) == HEADER_SIZE                       # Ensure the header is exactly 512 bytes long

    open(out_bin, 'wb').write(header + body)                # 'wb' = write, binary. header + body concatenates the two byte strings
    print(f"{len(body)} bytes, sha256={digest.hex()}")

if __name__ == '__main__':
    if len(sys.argv) != 4:
        print("usage: sign_image.py <app.bin> <out.bin> <version>")
        sys.exit(1)
    build(sys.argv[1], sys.argv[2], int(sys.argv[3]))
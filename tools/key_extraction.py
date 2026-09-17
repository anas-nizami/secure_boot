import sys
from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives.serialization import Encoding, PublicFormat, load_pem_public_key
from pathlib import Path

def getBytes():
    file_path = 'keys/public-key.pem'
    try:
        with open(file_path, 'rb') as f:
            public_key_bytes = f.read()
        return public_key_bytes
    except FileNotFoundError:
        print(f"Error: File not found - {file_path}")
        sys.exit(1)
    except IOError:
        print(f"Error: Unable to read file - {file_path}")
        sys.exit(1)

def generate_header(pubkey):
        current_dir = Path.cwd()
        print(f"Generating header file in {current_dir / 'bootloader' / 'inc' / 'pubkey.h'}")
        with open(current_dir / 'bootloader' / 'inc' / 'pubkey.h', 'w') as f:
            f.write("/* Generated from keys/public-key.pem - do not edit */\n")
            f.write(f"/* fingerprint: {pubkey[:4].hex()} */\n\n")
            f.write("#ifndef PUBKEY_H\n#define PUBKEY_H\n\n#include <stdint.h>\n\n")
            f.write("static const uint8_t g_pubkey[64] = {\n")
            for i in range(0, 64, 8):
                row = ", ".join(f"0x{b:02x}" for b in pubkey[i:i+8])
                f.write(f"    {row},\n")
            f.write("};\n\n#endif\n")

if __name__ == '__main__':

    public_key_bytes = getBytes()
    public_key = load_pem_public_key(public_key_bytes, backend=default_backend())

    raw_key = public_key.public_bytes(Encoding.X962, PublicFormat.UncompressedPoint)
    print(f"Raw key: {raw_key.hex()}")

    assert raw_key[0] == 0x04, f"expected uncompressed point, got 0x{raw_key[0]:02x}"
    pubkey = raw_key[1:]
    print(f"Public key: {pubkey.hex()}")
    assert len(pubkey) == 64, f"expected 64 bytes, got {len(pubkey)}"

    # Validate the public key is on the P-256 curve
    # The values p and B are taken from the NIST P-256 curve specification
    # Run this commnd in termail to get the values of p and B: openssl ecparam -name prime256v1 -param_enc explicit -text -noout
    x = int.from_bytes(pubkey[:32], 'big')
    y = int.from_bytes(pubkey[32:], 'big')
    p = 0xffffffff00000001000000000000000000000000ffffffffffffffffffffffff
    b = 0x5ac635d8aa3a93e7b3ebbd55769886bc651d06b0cc53b0f63bce3c3e27d2604b
    assert (y*y - (x*x*x - 3*x + b)) % p == 0, "point is not on the P-256 curve"

    generate_header(pubkey)

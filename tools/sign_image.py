#!/usr/bin/env python3
import hashlib, struct, sys
from cryptography.hazmat.backends import default_backend
from cryptography.exceptions import InvalidSignature
from cryptography.hazmat.primitives.serialization import load_pem_private_key
from cryptography.hazmat.primitives.asymmetric.utils import decode_dss_signature
from cryptography.hazmat.primitives.asymmetric import ec, utils
from cryptography.hazmat.primitives import hashes
from pathlib import Path

MAGIC = 0x4E495A41
HEADER_SIZE = 512

def read_file(file_path):
    try:
        with open(file_path, 'rb') as f:
            return f.read()
    except FileNotFoundError:
        print(f"Error: File not found - {file_path}")
        sys.exit(1)
    except IOError:
        print(f"Error: Unable to read file - {file_path}")
        sys.exit(1)


def write_file(file_name, data):
    try:
        with open(file_name, 'wb') as f:
            f.write(data)
            print(f"Written {len(data)} bytes to {file_name}")
    except IOError:
        print(f"Error: Unable to write file - {file_name}")
        sys.exit(1)

def build(version, key_file):
    REPO_ROOT = Path(__file__).resolve().parent.parent

    body = read_file(REPO_ROOT / 'bootloader' / 'Debug' / 'Secure_Boot.bin')  # Read the application binary file in binary mode, without the b in rb, it would read it in text mode, which could cause issues with binary data.
    out_bin = REPO_ROOT / 'flash_image' / key_file.replace('.pem', f'_{version}.bin')  # Replace the .pem extension with .bin for the output file name

    private_key_file = read_file(REPO_ROOT / 'keys' / key_file)  # Read the private key file in binary mode
    private_key = load_pem_private_key(private_key_file, password=None, backend=default_backend())  # Load the private key from the PEM file

    # The digest is a fixed-size output that uniquely represents the input data, ensuring integrity and authenticity.
    digest = hashlib.sha256(body).digest()  # Hash the whole file in one call. .digest() gives 32 raw bytes. This will be used to sign the image and verify below.

    """
    Sign the digest with the private key. 
        1. hashes.SHA256()  - is used to specify which hash algorithm is being used.
        2. utils.Prehashe() - a wrapper saying "the data I'm handing you is already a digest, don't hash it again." The hashing operation is done in the previous line, so we don't want to hash it again.
        3. ec.ECDSA - the signature algorithm. ECDSA needs to know which hash was used
        4. private_key.sign() - the actual signing operation, which takes the digest and the signature algorithm(ECDSA) as input and returns the signature in DER format.
    """
    der = private_key.sign(digest, ec.ECDSA(utils.Prehashed(hashes.SHA256())))
    r, s = decode_dss_signature(der) # parses it back to two Python integers
    signature = r.to_bytes(32, 'big') + s.to_bytes(32, 'big')
    assert len(signature) == 64, f"expected 64, got {len(signature)}"

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
    64s         64 bytes string - The signed digest (r and s values concatenated)    
    """
    header = struct.pack('<IIII32s64s',
                         MAGIC, version, len(body), 0,
                         digest, signature)

    try:
        private_key.public_key().verify(der, digest, ec.ECDSA(utils.Prehashed(hashes.SHA256())))
    except InvalidSignature:
        print("Error: signature failed to verify after signing")
        sys.exit(1)
    
    header += b'\x00' * (HEADER_SIZE - len(header))
    assert len(header) == HEADER_SIZE                 # Ensure the header is exactly 512 bytes long

    write_file(out_bin, header + body)                # 'wb' = write, binary. header + body concatenates the two byte strings
    print(f"{len(body)} bytes, sha256={digest.hex()}")

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print("usage: sign_image.py  <version> <key.pem>")
        sys.exit(1)
    build(int(sys.argv[1]), sys.argv[2])
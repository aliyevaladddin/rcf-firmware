#!/usr/bin/env python3
# [RCF:RESTRICTED] — Factory provisioning tool (Integrated with rcf-signer)

import argparse
import hashlib
import struct
import os
import time
import subprocess
from cryptography.hazmat.primitives import hashes, serialization
from cryptography.hazmat.primitives.asymmetric import ed25519, x25519

VAULT_MAGIC = b"RCF_VAULT_MARKER"

def compute_code_fingerprint(binary: bytes) -> bytes:
    return hashlib.sha256(binary).digest()

def find_vault_offset(binary: bytes) -> int:
    offset = binary.find(VAULT_MAGIC)
    if offset == -1:
        # If marker not found, append to end (assuming kernel will find it)
        return len(binary)
    return offset

def construct_vault(device_pubkey, device_privkey, genesis, initial_license):
    # Simplified vault structure for ARM64 dOS
    vault = bytearray()
    vault += VAULT_MAGIC
    vault += device_pubkey # 32 bytes (X25519)
    vault += genesis['entropy'] # 16 bytes
    vault += struct.pack('<Q', genesis['timestamp'])
    vault += genesis['self_hash'] # 32 bytes
    vault += initial_license[:128] # Truncated for demo vault size
    return bytes(vault)

def sign_with_native_tool(binary_path, output_path):
    # Use environment variable for the signer path, or a relative path as fallback
    signer_path = os.environ.get("RCF_SIGNER_PATH")
    if not signer_path:
        # Fallback to sibling directory SentinelCortex relative to the script location
        script_dir = os.path.dirname(os.path.abspath(__file__))
        signer_path = os.path.abspath(os.path.join(script_dir, "../../SentinelCortex/bin/rcf-signer"))

    if not os.path.exists(signer_path):
        raise FileNotFoundError(f"Native signer not found at {signer_path}")
    
    print(f"[RCF] Calling native signer: {signer_path}")
    result = subprocess.run([signer_path, "sign", binary_path, output_path], capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Signer Error: {result.stderr}")
        raise RuntimeError("Native signing failed")
    print(result.stdout)

def provision_device(device_binary: bytes, device_uid: str) -> bytes:
    print("[RCF] Starting provisioning workflow...")
    
    # 1. Generate device identity (X25519 for UTP/Encryption)
    device_priv = x25519.X25519PrivateKey.generate()
    device_pub = device_priv.public_key()
    device_pubkey_bytes = device_pub.public_bytes(
        encoding=serialization.Encoding.Raw,
        format=serialization.PublicFormat.Raw
    )
    
    # 2. Create genesis timechain
    genesis = {
        'timestamp': int(time.time()),
        'entropy': os.urandom(16),
    }
    genesis['self_hash'] = hashlib.sha256(
        struct.pack('<Q', genesis['timestamp']) + genesis['entropy']
    ).digest()
    
    # 3. Create initial license (Mock)
    license_blob = os.urandom(256)
    
    # 4. Construct provisioning image
    vault_data = construct_vault(
        device_pubkey=device_pubkey_bytes,
        device_privkey=None, # In real HSM, private key stays inside
        genesis=genesis,
        initial_license=license_blob
    )
    
    print(f"[RCF] Vault constructed. Size: {len(vault_data)} bytes.")
    
    provisioned_image = bytearray(device_binary)
    vault_offset = find_vault_offset(device_binary)
    
    if vault_offset == len(device_binary):
        print("[WARN] Vault marker not found. Appending vault to end of binary.")
        provisioned_image += vault_data
    else:
        print(f"[INFO] Vault marker found at offset 0x{vault_offset:08X}")
        provisioned_image[vault_offset:vault_offset + len(vault_data)] = vault_data
        
    return bytes(provisioned_image), device_pubkey_bytes, genesis['self_hash']

def main():
    parser = argparse.ArgumentParser(description='RCF-Lume Factory Provisioning')
    parser.add_argument('--device', required=True, help='Path to device binary')
    parser.add_argument('--output', required=True, help='Output provisioned binary')
    
    args = parser.parse_args()
    
    if not os.path.exists(args.device):
        print(f"Error: Device binary {args.device} not found.")
        return

    with open(args.device, 'rb') as f:
        device_binary = f.read()
    
    # Process 1: Provision (Vault injection)
    provisioned, pubkey, gen_hash = provision_device(device_binary, args.uid)
    
    tmp_prov = "bin/tmp_provisioned.bin"
    os.makedirs("bin", exist_ok=True)
    with open(tmp_prov, 'wb') as f:
        f.write(provisioned)
    
    # Process 2: Sign (Native PQC)
    try:
        sign_with_native_tool(tmp_prov, args.output)
        print(f"\n[SUCCESS] Final provisioned and signed image: {args.output}")
        print(f"Device Pubkey: {pubkey.hex().upper()}")
        print(f"Genesis Hash:  {gen_hash.hex().upper()}")
    except Exception as e:
        print(f"[!!] Provisioning Failed: {e}")
    finally:
        if os.path.exists(tmp_prov):
            os.remove(tmp_prov)

if __name__ == '__main__':
    main()
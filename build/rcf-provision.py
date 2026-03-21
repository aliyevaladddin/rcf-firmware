#!/usr/bin/env python3
# [RCF:RESTRICTED] — Factory provisioning tool

import argparse
import hashlib
import struct
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric import ed25519, x25519
from cryptography.hazmat.primitives.ciphers.aead import AESGCM
import serial

def provision_device(device_binary: bytes, hsm_connection) -> bytes:
    """
    Factory provisioning workflow:
    1. Generate device unique keys (Curve25519, Ed25519)
    2. Create genesis timechain block
    3. Issue initial license (BASELINE tier)
    4. Flash to device with HSM-signed certificate
    """
    
    # 1. Generate device identity
    device_priv = x25519.X25519PrivateKey.generate()
    device_pub = device_priv.public_key()
    device_pubkey_bytes = device_pub.public_bytes(
        encoding=serialization.Encoding.Raw,
        format=serialization.PublicFormat.Raw
    )
    
    # 2. Create genesis timechain
    genesis = {
        'timestamp': int(time.time()),
        'monotonic_counter': 0,
        'entropy': os.urandom(16),
        'prev_hash': b'\x00' * 32,  # Genesis has no predecessor
    }
    genesis['self_hash'] = hashlib.sha256(
        struct.pack('<Q', genesis['timestamp']) +
        genesis['entropy']
    ).digest()
    
    # 3. Create initial license (BASELINE)
    license_payload = {
        'code_fingerprint': compute_code_fingerprint(device_binary),
        'issue_timestamp': genesis['timestamp'],
        'expiration_timestamp': genesis['timestamp'] + (365 * 24 * 3600),  # 1 year
        'feature_flags': 0x01,  # BASELINE only
        'device_binding': device_uid,  # From STM32 UID read via SWD
        'audit_anchor': genesis['self_hash'],
    }
    
    # Encrypt and sign license
    license_blob = encrypt_and_sign_license(license_payload, hsm_connection)
    
    # 4. Construct provisioning image
    provisioned_image = bytearray(device_binary)
    
    # Inject vault at known offset
    vault_offset = find_vault_offset(device_binary)
    vault_data = construct_vault(
        device_pubkey=device_pubkey_bytes,
        device_privkey=device_priv.private_bytes(...),
        genesis=genesis,
        initial_license=license_blob
    )
    provisioned_image[vault_offset:vault_offset + len(vault_data)] = vault_data
    
    # 5. HSM signature of entire image
    final_image = hsm_connection.sign_image(bytes(provisioned_image))
    
    return final_image

def main():
    parser = argparse.ArgumentParser(description='RCF-Lume Factory Provisioning')
    parser.add_argument('--device', required=True, help='Path to device binary')
    parser.add_argument('--hsm', required=True, choices=['yubihsm', 'luna', 'soft'])
    parser.add_argument('--output', required=True, help='Output provisioned binary')
    
    args = parser.parse_args()
    
    with open(args.device, 'rb') as f:
        device_binary = f.read()
    
    hsm = HSMFactory.create(args.hsm)
    provisioned = provision_device(device_binary, hsm)
    
    with open(args.output, 'wb') as f:
        f.write(provisioned)
    
    print(f"Provisioned device image: {args.output}")
    print(f"Device pubkey: {provisioned.device_pubkey.hex()}")
    print(f"Genesis hash: {provisioned.genesis_hash.hex()}")

if __name__ == '__main__':
    main()
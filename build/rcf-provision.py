#!/usr/bin/env python3
# NOTICE: This file is protected under RCF-PL v1.3
# [RCF:RESTRICTED] — Factory provisioning tool (Integrated with rcf-signer)

import argparse
import hashlib
import json
import os
import struct
import subprocess
import sys
import time
from pathlib import Path

from cryptography.hazmat.primitives import serialization
from cryptography.hazmat.primitives.asymmetric import x25519

# ─── Constants ────────────────────────────────────────────────────────────────

VAULT_MAGIC       = b"RCF_VAULT_MARKER"
VAULT_VERSION     = 0x0103          # v1.3
LICENSE_SLOT_SIZE = 128
ENTROPY_SIZE      = 16

# rcf-signer lookup order (no hardcoded personal paths)
_SIGNER_CANDIDATES = [
    os.environ.get("RCF_SIGNER_PATH", ""),   # 1. env variable (preferred)
    "bin/rcf-signer",                          # 2. local bin/
    "rcf-signer",                              # 3. PATH
    "/usr/local/bin/rcf-signer",               # 4. system-wide install
]

# ─── Vault construction ───────────────────────────────────────────────────────

# [RCF:RESTRICTED]
def compute_code_fingerprint(binary: bytes) -> bytes:
    """SHA-256 fingerprint of the raw binary."""
    return hashlib.sha256(binary).digest()


def find_vault_offset(binary: bytes) -> int:
    """Locate the RCF_VAULT_MARKER placeholder in the binary."""
    offset = binary.find(VAULT_MAGIC)
    return offset if offset != -1 else len(binary)


def construct_vault(
    device_pubkey: bytes,
    genesis: dict,
    initial_license: bytes,
) -> bytes:
    """
    Build the provisioning vault blob.

    Layout (ARM64 dOS, little-endian):
      [0:16]   VAULT_MAGIC
      [16:18]  vault_version (u16 LE)
      [18:50]  device_pubkey  (X25519, 32 bytes)
      [50:66]  entropy        (16 bytes)
      [66:74]  timestamp      (u64 LE)
      [74:106] genesis_hash   (SHA-256, 32 bytes)
      [106:234] license_slot  (128 bytes, zero-padded)
    """
    license_padded = initial_license[:LICENSE_SLOT_SIZE].ljust(LICENSE_SLOT_SIZE, b'\x00')

    vault = bytearray()
    vault += VAULT_MAGIC
    vault += struct.pack('<H', VAULT_VERSION)
    vault += device_pubkey
    vault += genesis['entropy']
    vault += struct.pack('<Q', genesis['timestamp'])
    vault += genesis['self_hash']
    vault += license_padded
    return bytes(vault)


# ─── Signing ──────────────────────────────────────────────────────────────────

def _resolve_signer() -> str:
    """Find rcf-signer binary using the lookup order above."""
    for candidate in _SIGNER_CANDIDATES:
        if candidate and Path(candidate).is_file():
            return str(Path(candidate).resolve())
    raise FileNotFoundError(
        "rcf-signer not found. Set RCF_SIGNER_PATH env variable or place it in bin/rcf-signer."
    )


def sign_with_native_tool(binary_path: str, output_path: str) -> None:
    """Invoke the native PQC signer (Dilithium2) via subprocess."""
    signer = _resolve_signer()
    print(f"[RCF] Calling native signer: {signer}")

    result = subprocess.run(
        [signer, "sign", binary_path, output_path],
        capture_output=True,
        text=True,
    )

    if result.stdout:
        print(result.stdout.strip())
    if result.returncode != 0:
        print(f"[!!] Signer stderr: {result.stderr.strip()}", file=sys.stderr)
        raise RuntimeError(f"Native signing failed (exit {result.returncode})")


# ─── Provisioning workflow ────────────────────────────────────────────────────

# [RCF:RESTRICTED]
def provision_device(
    device_binary: bytes,
    device_uid: str,
    dry_run: bool = False,
) -> tuple[bytes, bytes, bytes]:
    """
    Full provisioning pipeline:
      1. Generate device X25519 identity keypair
      2. Create genesis timechain
      3. Construct vault blob
      4. Inject vault into binary

    Returns:
      (provisioned_image, device_pubkey_bytes, genesis_hash)
    """
    print(f"[RCF] Starting provisioning — UID: {device_uid}")

    # 1. Device identity (X25519 — used for UTP/encryption)
    device_priv = x25519.X25519PrivateKey.generate()
    device_pub  = device_priv.public_key()
    device_pubkey_bytes = device_pub.public_bytes(
        encoding=serialization.Encoding.Raw,
        format=serialization.PublicFormat.Raw,
    )

    # 2. Genesis timechain
    entropy   = os.urandom(ENTROPY_SIZE)
    timestamp = int(time.time())
    genesis_hash = hashlib.sha256(
        struct.pack('<Q', timestamp) + entropy
    ).digest()

    genesis = {
        'timestamp': timestamp,
        'entropy':   entropy,
        'self_hash': genesis_hash,
    }

    # 3. License slot — in production, issued by RCF License Authority
    #    In factory mode, a provisioning token is fetched from the HSM.
    license_blob = os.urandom(256)

    # 4. Vault construction
    vault_data = construct_vault(
        device_pubkey=device_pubkey_bytes,
        genesis=genesis,
        initial_license=license_blob,
    )
    print(f"[RCF] Vault constructed — {len(vault_data)} bytes, version 0x{VAULT_VERSION:04X}")

    if dry_run:
        print("[RCF] Dry-run mode — binary not modified.")
        return device_binary, device_pubkey_bytes, genesis_hash

    # 5. Inject vault into binary
    provisioned = bytearray(device_binary)
    offset      = find_vault_offset(device_binary)

    if offset == len(device_binary):
        print("[WARN] Vault marker not found — appending vault to end of binary.")
        provisioned += vault_data
    else:
        print(f"[INFO] Vault marker at offset 0x{offset:08X} — injecting.")
        provisioned[offset : offset + len(vault_data)] = vault_data

    return bytes(provisioned), device_pubkey_bytes, genesis_hash


def write_manifest(
    output_path: str,
    device_uid: str,
    pubkey: bytes,
    genesis_hash: bytes,
) -> None:
    """Write a JSON provisioning manifest alongside the output binary."""
    manifest = {
        "rcf_version":  "1.3",
        "device_uid":   device_uid,
        "provisioned_at": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
        "device_pubkey": pubkey.hex().upper(),
        "genesis_hash":  genesis_hash.hex().upper(),
        "output_binary": str(Path(output_path).name),
    }
    manifest_path = Path(output_path).with_suffix(".manifest.json")
    manifest_path.write_text(json.dumps(manifest, indent=2))
    print(f"[RCF] Manifest written: {manifest_path}")


# ─── CLI ─────────────────────────────────────────────────────────────────────

def main() -> None:
    parser = argparse.ArgumentParser(
        description="RCF-Lume Factory Provisioning Tool v1.3",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=(
            "Environment variables:\n"
            "  RCF_SIGNER_PATH   Path to rcf-signer binary\n\n"
            "Example:\n"
            "  RCF_SIGNER_PATH=./bin/rcf-signer \\\n"
            "    python rcf-provision.py \\\n"
            "      --device build/aurora.bin \\\n"
            "      --output build/aurora.provisioned.bin \\\n"
            "      --uid DEVICE-ARM64-001"
        )
    )
    parser.add_argument("--device",   required=True, help="Path to raw device binary")
    parser.add_argument("--output",   required=True, help="Output provisioned binary path")
    parser.add_argument("--uid",      default="DEVICE-ARM64-AUTO", help="Device UID string")
    parser.add_argument("--dry-run",  action="store_true", help="Simulate provisioning without writing")
    parser.add_argument("--no-sign",  action="store_true", help="Skip PQC signing step")
    parser.add_argument("--manifest", action="store_true", help="Write JSON provisioning manifest")
    args = parser.parse_args()

    device_path = Path(args.device)
    if not device_path.is_file():
        print(f"[!!] Device binary not found: {args.device}", file=sys.stderr)
        sys.exit(1)

    device_binary = device_path.read_bytes()
    print(f"[RCF] Loaded binary: {args.device} ({len(device_binary):,} bytes)")

    # Step 1 — Provision (vault injection)
    provisioned, pubkey, gen_hash = provision_device(
        device_binary=device_binary,
        device_uid=args.uid,
        dry_run=args.dry_run,
    )

    if args.dry_run:
        print(f"\n[DRY-RUN] No files written.")
        print(f"  Device Pubkey : {pubkey.hex().upper()}")
        print(f"  Genesis Hash  : {gen_hash.hex().upper()}")
        return

    # Step 2 — Write provisional image to temp file
    output_path = Path(args.output)
    output_path.parent.mkdir(parents=True, exist_ok=True)
    tmp_path = output_path.with_suffix(".tmp.bin")
    tmp_path.write_bytes(provisioned)

    # Step 3 — PQC signing
    if not args.no_sign:
        try:
            sign_with_native_tool(str(tmp_path), str(output_path))
        except Exception as e:
            print(f"[!!] Signing failed: {e}", file=sys.stderr)
            tmp_path.unlink(missing_ok=True)
            sys.exit(1)
        finally:
            tmp_path.unlink(missing_ok=True)
    else:
        tmp_path.rename(output_path)
        print(f"[WARN] Signing skipped (--no-sign).")

    # Step 4 — Manifest
    if args.manifest:
        write_manifest(str(output_path), args.uid, pubkey, gen_hash)

    print(f"\n[SUCCESS] Provisioned image: {output_path}")
    print(f"  Device Pubkey : {pubkey.hex().upper()}")
    print(f"  Genesis Hash  : {gen_hash.hex().upper()}")


if __name__ == "__main__":
    main()
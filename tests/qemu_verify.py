# [RCF:NOTICE][RCF:PROTECTED]
# QEMU Verification Script for RCF v1.3 CI.

import subprocess
import time
import sys

def run_qemu_test(elf_path, timeout=10):
    print(f"[RCF-TEST] Starting QEMU simulation for {elf_path}...")
    
    # Run QEMU with non-graphical output on serial
    cmd = [
        "qemu-system-arm",
        "-M", "netduinoplus2",
        "-cpu", "cortex-m4",
        "-kernel", elf_path,
        "-nographic",
        "-serial", "mon:stdio"
    ]
    
    process = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    start_time = time.time()
    success = False
    
    try:
        while time.time() - start_time < timeout:
            line = process.stdout.readline()
            if not line:
                break
            
            print(f"QEMU: {line.strip()}")
            
            # Look for success markers (Audit events or VM ready signal)
            # In RC1, we expect EVENT_VM_VERIFY_OK (0x1312) signal or "A-VM Initialized"
            if "RCF v1.3" in line or "VERIFY_OK" in line or "0x1312" in line:
                print("[RCF-TEST] SUCCESS: Firmware initialized correctly.")
                success = True
                break
                
            if "PILL_OFF" in line or "TAMPER" in line:
                print("[RCF-TEST] FAILURE: Emergency Pill-Off triggered!")
                break
                
    finally:
        process.kill()
        
    return success

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 qemu_verify.py <elf_path>")
        sys.exit(1)
        
    result = run_qemu_test(sys.argv[1])
    if result:
        print("[RCF-TEST] CI Verification PASSED.")
        sys.exit(0)
    else:
        print("[RCF-TEST] CI Verification FAILED.")
        sys.exit(1)

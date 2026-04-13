# [RCF:PROTECTED]
# integrate_qemu.py — Aurora Access Bridge Orchestrator
# Connects ARM64 (dOS) and STM32 (HSM) QEMU instances via UNIX sockets.

import subprocess
import os
import signal
import sys
import threading
import time

# --- Configuration ---
SOCKET_PATH = "/tmp/rcf_bridge.sock"
STM32_ELF   = ".build/rcf-lume-rc1.elf"
ARM64_ELF   = "../ARM64-core/aurora_kernel_qemu.elf"

# --- Colors for Output ---
CLR_ARM64 = "\033[94m" # Blue
CLR_STM32 = "\033[92m" # Green
CLR_RESET = "\033[0m"

processes = []

def cleanup():
    """Cleanup processes and sockets."""
    for p in processes:
        try:
            p.terminate()
        except:
            pass
    if os.path.exists(SOCKET_PATH):
        os.remove(SOCKET_PATH)

def monitor_output(process, name, color):
    """Thread function to read and print process output."""
    while True:
        line = process.stdout.readline()
        if not line:
            break
        print(f"{color}[{name}]{CLR_RESET} {line.strip()}")

def signal_handler(sig, frame):
    print("\n[ORCHESTRATOR] Shutting down simulation...")
    cleanup()
    sys.exit(0)

def main():
    if os.path.exists(SOCKET_PATH):
        os.remove(SOCKET_PATH)

    print("🚀 [ORCHESTRATOR] Initializing Aurora Access Bridge...")

    # 1. Start STM32 (HSM) - acts as Serial Server
    # We redirect its serial to the socket.
    print(f"📦 [HSM] Starting STM32 Bunker ({STM32_ELF})...")
    stm32_cmd = [
        "qemu-system-arm",
        "-M", "netduinoplus2",
        "-cpu", "cortex-m4",
        "-kernel", STM32_ELF,
        "-nographic",
        "-serial", f"unix:{SOCKET_PATH},server,nowait"
    ]
    
    p_stm32 = subprocess.Popen(stm32_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    processes.append(p_stm32)
    threading.Thread(target=monitor_output, args=(p_stm32, "HSM", CLR_STM32), daemon=True).start()

    # Wait a moment for socket to be ready
    time.sleep(1)

    # 2. Start ARM64 (dOS) - acts as Serial Client
    # UART0 (mon:stdio) for console, UART1 (unix:socket) for Bridge
    print(f"🧠 [dOS] Starting ARM64 Core ({ARM64_ELF})...")
    arm64_cmd = [
        "qemu-system-aarch64",
        "-M", "virt",
        "-cpu", "cortex-a72",
        "-kernel", ARM64_ELF,
        "-nographic",
        "-serial", "mon:stdio",
        "-serial", f"unix:{SOCKET_PATH}"
    ]
    
    p_arm64 = subprocess.Popen(arm64_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    processes.append(p_arm64)
    threading.Thread(target=monitor_output, args=(p_arm64, "dOS", CL_ARM64), daemon=True).start()

    print("✨ [ORCHESTRATOR] Bridge Established. Monitoring traffic...")
    
    try:
        while True:
            # Check if processes are alive
            if p_stm32.poll() is not None or p_arm64.poll() is not None:
                break
            time.sleep(1)
    except KeyboardInterrupt:
        pass
    finally:
        cleanup()

if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal_handler)
    
    # Fix for CL_ARM64 typo in monitor thread
    CL_ARM64 = CLR_ARM64
    
    main()

# [RCF:PROTECTED]
# integrate_qemu.py — Aurora Access Bridge Orchestrator
import subprocess, os, signal, sys, threading, time

SOCKET_PATH = "/tmp/rcf_bridge.sock"
STM32_ELF   = os.path.join(os.getcwd(), ".build/rcf-lume-rc1.elf")
# Force absolute path for Codespace environment
ARM64_ELF = "/workspaces/ARM64-core/aurora_kernel_qemu.elf"
if not os.path.exists(ARM64_ELF):
    # Fallback for local Mac or other environments
    ARM64_ELF = "../ARM64-core/aurora_kernel_qemu.elf"

# Colors
CLR_ARM64 = "\033[94m" # Blue
CLR_STM32 = "\033[92m" # Green
CLR_RESET = "\033[0m"

processes = []

def cleanup():
    for p in processes:
        try: p.terminate()
        except: pass
    if os.path.exists(SOCKET_PATH): os.remove(SOCKET_PATH)

def monitor_output(process, name, color):
    for line in process.stdout:
        if not line: break
        decoded_line = line.decode('utf-8', errors='replace').strip()
        print(f"{color}[{name}]{CLR_RESET} {decoded_line}")

def main():
    if os.path.exists(SOCKET_PATH): os.remove(SOCKET_PATH)
    print("🚀 [ORCHESTRATOR] Initializing Aurora Access Bridge...")

    # 1. Start STM32 (HSM)
    stm32_cmd = ["qemu-system-arm", "-M", "netduinoplus2", "-cpu", "cortex-m4", "-kernel", STM32_ELF, "-nographic", "-serial", f"unix:{SOCKET_PATH},server,nowait"]
    p_stm32 = subprocess.Popen(stm32_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    processes.append(p_stm32)
    threading.Thread(target=monitor_output, args=(p_stm32, "HSM", CLR_STM32), daemon=True).start()

    time.sleep(1)

    # 2. Start ARM64 (dOS)
    arm64_cmd = ["qemu-system-aarch64", "-M", "virt", "-cpu", "cortex-a72", "-kernel", ARM64_ELF, "-nographic", "-serial", "mon:stdio", "-serial", f"unix:{SOCKET_PATH}"]
    p_arm64 = subprocess.Popen(arm64_cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    processes.append(p_arm64)
    threading.Thread(target=monitor_output, args=(p_arm64, "dOS", CLR_ARM64), daemon=True).start()

    print("✨ [ORCHESTRATOR] Bridge Established. Monitoring traffic...")
    try:
        while True:
            if p_stm32.poll() is not None or p_arm64.poll() is not None: break
            time.sleep(1)
    except KeyboardInterrupt: pass
    finally: cleanup()

if __name__ == "__main__":
    signal.signal(signal.SIGINT, lambda s, f: (cleanup(), sys.exit(0)))
    main()

/* 
 * [RCF:NOTICE][RCF:PUBLIC]
 * Bare Metal Newlib Support — System Call Stubs.
 * NOTICE: This file is protected under RCF-PL v1.3
 */

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

/* Exit — infinite loop (no OS to return to) */
void _exit(int status) { 
    (void)status; 
#ifdef RCF_VM_CI_MODE
    /* ARM Semihosting exit for QEMU */
    register int r0 __asm__("r0") = 0x18; // SYS_EXIT
    register int r1 __asm__("r1") = (status == 0) ? 0x20026 : 0x20024;
    __asm__ volatile("bkpt 0xab" : : "r"(r0), "r"(r1) : "memory");
#endif
    while(1); 
}

/* Process control — bare metal has no processes */
int _kill(int pid, int sig) { 
    (void)pid; (void)sig; 
    errno = EINVAL; 
    return -1; 
}

int _getpid(void) { 
    return 1; 
}

/* Heap — simple bump allocator */
caddr_t _sbrk(int incr) {
    extern char end asm("end");
    extern char _estack; /* Top of stack = heap limit (defined in LD) */
    static char *heap_end;
    char *prev_heap_end;
    
    if (heap_end == 0) heap_end = &end;
    prev_heap_end = heap_end;
    
    /* 16KB stack reserve to prevent collision */
    if (heap_end + incr > &_estack - 0x4000) { 
        errno = ENOMEM;
        return (caddr_t)-1;
    }
    
    heap_end += incr;
    return (caddr_t)prev_heap_end;
}

/* File I/O — minimal stubs (no filesystem) */
/* _write is now implemented in hal_stubs.c for Dual-UART support */
#if 0
int _write(int file, char *ptr, int len) { 
#ifdef RCF_VM_CI_MODE
    /* Direct UART1 access for QEMU console (STM32F4 Standard) */
    for (int i = 0; i < len; i++) {
        *(volatile uint32_t*)0x40011004 = (uint32_t)ptr[i];
    }
    return len;
#else
    (void)file; (void)ptr; 
    return len; 
#endif
}
#endif

int _read(int file, char *ptr, int len) { 
    (void)file; (void)ptr; 
    (void)len; 
    return 0; 
}

int _close(int file) { 
    (void)file; 
    return -1; 
}

int _fstat(int file, struct stat *st) { 
    (void)file; 
    st->st_mode = S_IFCHR; 
    return 0; 
}

int _isatty(int file) { 
    (void)file; 
    return 1; 
}

int _lseek(int file, int ptr, int dir) { 
    (void)file; (void)ptr; (void)dir; 
    return 0; 
}

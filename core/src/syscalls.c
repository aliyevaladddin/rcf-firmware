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
int _write(int file, char *ptr, int len) { 
    (void)file; (void)ptr; 
    return len; 
}

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

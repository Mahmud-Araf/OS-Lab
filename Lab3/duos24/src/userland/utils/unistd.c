/*
 * Copyright (c) 2022 
 * Computer Science and Engineering, University of Dhaka
 * Credit: CSE Batch 25 (starter) and Prof. Mosaddek Tushar
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE UNIVERSITY AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE UNIVERSITY OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
 
#include <unistd.h>
#include <syscall_def.h>
#include <kstdio.h>

void ufopen(char *name, uint8_t t_access, uint32_t *op_addr)
{
	__asm volatile(
		"mov r0, %[x]\n"
		"mov r1, %[y]\n"
		:
		: [x] "r"(name), [y] "r"(t_access));
	__asm volatile(
		"mov r2, %[x]\n"
		:
		: [x] "r"(op_addr));

	__asm volatile("PUSH {r4-r11, ip, lr}");
	__asm volatile("svc %0" : : "i"(SYS_open));

	__asm volatile("POP {r4-r11, ip, lr}");
}

void ufclose(uint32_t *op_addr)
{
	__asm volatile(
		"mov r0, %[x]\n"
		:
		: [x] "r"(op_addr));
	__asm volatile("PUSH {r4-r11, ip, lr}");
	__asm volatile("svc %0" : : "i"(SYS_close));

	__asm volatile("POP {r4-r11, ip, lr}");
}

void ureboot(void) {
	__asm volatile ("PUSH {r4-r11, ip, lr}");
	__asm volatile("svc %0" : : "i" (SYS_reboot));
	__asm volatile ("POP {r4-r11, ip, lr}");
}

void uread(uint8_t fd, char **data, uint32_t size) {
	__asm volatile (
		"mov r0, %[x]\n"
		"mov r1, %[y]\n"
		:
		: [x] "r" (fd), [y] "r" (data)
	);
	__asm volatile (
		"mov r2, %[x]\n"
		:
		: [x] "r" (size)
	);
	__asm volatile ("PUSH {r4-r11, ip, lr}");
	__asm volatile("svc %0" : : "i" (SYS_read));
	__asm volatile ("POP {r4-r11, ip, lr}");
}

void uwrite(uint8_t fd, char *data) {
	__asm volatile (
		"mov r0, %[x]\n"
		"mov r1, %[y]\n"
		:
		: [x] "r" (fd), [y] "r" (data)
	);
    __asm volatile("stmdb r13!, {r5}");

	__asm volatile ("PUSH {r4-r11, ip, lr}");
	__asm volatile("svc %0" : : "i" (SYS_write));
	__asm volatile ("POP {r4-r11, ip, lr}");
}


void uyield(void)
{
	__asm volatile("svc %0" : : "i"(SYS_yield));
}

void utask_exit(void) {
    __asm volatile("PUSH {r5}");
    __asm volatile ("PUSH {r4-r11, ip, lr}");
	__asm volatile("svc %0" : : "i" (SYS__exit));
	__asm volatile ("POP {r4-r11, ip, lr}");
    uyield();
}

uint32_t ugetpid(void) {
    unsigned int pid = 0;
    __asm volatile("mov r5, %[v]": : [v] "r" (&pid));
    __asm volatile("PUSH {r5}");

    __asm volatile ("PUSH {r4-r11, ip, lr}");
	__asm volatile("svc %0" : : "i" (SYS_getpid));
	__asm volatile ("POP {r4-r11, ip, lr}");
    return (uint16_t) pid;
}

void ustart_task(uint32_t psp) {
	__asm volatile ("MOV R0, %0"
		:
		:"r" (psp)
	);
	__asm volatile ("PUSH {r4-r11, ip, lr}");
	__asm volatile("svc %0" : : "i" (SYS_start));
	__asm volatile ("POP {r4-r11, ip, lr}");
}


void *umalloc(uint32_t size) {
    void *ptr;
    
    // Preserve all registers that might be clobbered
    __asm volatile(
        "mov r0, %[size]\n"        // Move size to r0
        "push {r4-r11, ip, lr}\n"  // Save context
        "svc %[syscall]\n"         // Make system call
        "mov %[result], r0\n"      // Get result immediately
        "pop {r4-r11, ip, lr}\n"   // Restore context
        : [result] "=r" (ptr)      // Output: ptr gets r0
        : [size] "r" (size),       // Input: size parameter
          [syscall] "i" (SYS_malloc)
        : "r0", "memory"           // Clobbers: r0 and memory
    );
    
    // Debug print the address we received
    kprintf("umalloc: received address 0x%x\n", (unsigned int)ptr);
    
    return ptr;
}

int ufree(void *ptr) {
    int result;
    
    // Use the same pattern as umalloc for consistency
    __asm volatile(
        "mov r0, %[ptr]\n"         // Move ptr to r0
        "push {r4-r11, ip, lr}\n"  // Save context
        "svc %[syscall]\n"         // Make system call
        "mov %[result], r0\n"      // Get result immediately
        "pop {r4-r11, ip, lr}\n"   // Restore context
        : [result] "=r" (result)   // Output: result gets r0
        : [ptr] "r" (ptr),         // Input: ptr parameter
          [syscall] "i" (SYS_free)
        : "r0", "memory"           // Clobbers: r0 and memory
    );
    
    // Debug print the result
    kprintf("ufree: received status %d\n", result);
    
    return result;
}


int ufork(void) {
    int result;
    __asm volatile (
        "PUSH {r4-r11, ip, lr}\n"
        "svc %[v]\n"
        "POP {r4-r11, ip, lr}\n"
        "mov %[ret], r0"
        : [ret] "=r" (result)
        : [v] "i" (SYS_fork)
        : "memory"
    );
    return result;
}
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
#include <kunistd.h>
#include <cm4.h>
#include <dev_table.h>
#include <kmain.h>     /* For TASK_STACK_SIZE */
#include <schedule.h>  /* For READY state and queue_add */
#include <kmem.h>     /* For memory management functions */
#include <stddef.h>

/* Add your functions here */
extern uint32_t device_count;
extern uint32_t TASK_ID;

void __sys_start_task(uint32_t psp)
{
    __asm volatile("MOV R0, %0"
                   :
                   : "r"(psp));
    __asm volatile("LDMIA R0!,{R4-R11}");
    __asm volatile("MSR PSP, R0");
    __asm volatile("ISB 0xf" ::: "memory");
    __asm volatile("MOV LR, 0xFFFFFFFD");
    __asm volatile("BX LR");
}

int __sys_open(char *name, uint8_t t_access, uint32_t *op_addr)
{
    if (device_count >= MAX_DEVICES)
        return -1;
    kstrcpy((uint8_t*)device_list[device_count].name, (uint8_t*)name);
    device_list[device_count].t_access = t_access;
    device_list[device_count].op_addr = op_addr;
    device_list[device_count].t_ref = 1;
    device_count++;
    return 0;
}

void __sys_close(uint32_t *op_addr)
{
    uint32_t delete_index = -1;
    for (int i = 0; i < device_count; i++)
    {
        if (device_list[i].op_addr == op_addr)
        {
            device_list[i].t_ref--;
            if (device_list[i].t_ref == 0)
            {
                delete_index = i;
            }
        }
    }
    if (delete_index != -1)
    {
        for (int i = delete_index; i < device_count - 1; i++)
        {
            device_list[i] = device_list[i + 1];
        }
        device_count--;
    }
}

void __sys_reboot(void)
{
    SCB->AIRCR = 0x05FA0004;
}

int __sys_read(uint32_t fd, char **data, uint32_t size)
{
    if (fd == 0)
    {
        uint8_t buff[size + 1];
        _USART_READ(USART2, buff, size);
        buff[size] = '\0';
        kstrcpy((uint8_t*)*data, buff);
        return 0;
    }
    return -1;
}

int __sys_write(uint32_t fd, char *data, uint32_t size)
{
    if (fd == 1)
    {
        _USART_WRITE(USART2, (uint8_t*)data);
        return 0;
    }
    return -1;
}

void __sys_getpid(unsigned int *val, uint16_t value)
{
    *val = value;
    return;
}

void __sys_get_time(uint32_t *time)
{
    *time = __getTime();
}

int __sys_free(void *ptr)
{
    // Check if pointer is valid
    if (!ptr) {
        return -1;
    }

    // Call memory manager to free the memory and update memory accounting table
    return __kmem_free(ptr);
}

void *__sys_malloc(uint32_t size)
{
    // Validate size
    if (size == 0) {
        return NULL;
    }

    // Call kernel memory allocator to allocate memory
    void *ptr = __kmem_malloc(size);
    kprintf("__sys_malloc: received 0x%x from __kmem_malloc\n", (unsigned int)ptr);
    
    // Return allocated memory address or NULL if allocation failed
    return ptr;
}

int __sys_fork(TCB_TypeDef *parent_task)
{
    // 1. Create new TCB for child process
    static TCB_TypeDef child_tcb;
    
    // 2. Calculate new stack address for child (allocate after parent's stack)
    uint32_t *parent_stack_bottom = (uint32_t *)((uint32_t)parent_task->psp & ~(TASK_STACK_SIZE - 1));
    uint32_t *child_stack_start = parent_stack_bottom - TASK_STACK_SIZE;
    
    // 3. Copy parent's TCB to child
    child_tcb.magic_number = parent_task->magic_number;
    child_tcb.task_id = TASK_ID++; // Get new task ID
    child_tcb.status = READY;
    child_tcb.execution_time = 0;
    child_tcb.waiting_time = 0;
    child_tcb.digital_sinature = parent_task->digital_sinature;
    
    // 4. Calculate child's stack pointer offset
    uint32_t stack_offset = (uint32_t)child_stack_start - (uint32_t)parent_stack_bottom;
    child_tcb.psp = (uint32_t *)((uint32_t)parent_task->psp + stack_offset);
    
    // 5. Copy parent's stack to child's stack
    uint32_t stack_size = (uint32_t)parent_stack_bottom + TASK_STACK_SIZE - (uint32_t)parent_task->psp;
    for (uint32_t i = 0; i < stack_size / sizeof(uint32_t); i++) {
        child_stack_start[i] = parent_task->psp[i];
    }
    
    // 6. Add child to ready queue
    queue_add(&child_tcb);
    
    // 7. Set return values:
    // - Parent gets child's PID
    // - Child gets 0 (will be set when its context is restored)
    uint32_t *child_stack = child_tcb.psp;
    child_stack[6] = 0; // R0 will be 0 when child starts
    
    return child_tcb.task_id; // Return child's PID to parent
}


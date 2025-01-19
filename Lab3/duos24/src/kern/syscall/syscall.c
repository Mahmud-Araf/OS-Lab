#include <syscall.h>

void syscall(uint32_t *svc_args)
{
	uint16_t callno = ((char *)svc_args[6])[-2];
	uint32_t *op_addr;

	// Save r0 before any modifications
	uint32_t original_r0 = svc_args[0];
	
	switch (callno)
	{
	case SYS_open:
	{
		kprintf("Will call __sys_open\n");
		char *device_name = (char *)svc_args[0];
		uint8_t t_access = (uint8_t)svc_args[1];
		op_addr = (uint32_t *)svc_args[2];
		kprintf("wot :0\n");
		__sys_open(device_name, t_access, op_addr);
		break;
	}
	case SYS_close:
	{
		kprintf("Will call __sys_close\n");
		op_addr = (uint32_t *)svc_args[0];
		__sys_close(op_addr);
		break;
	}
	case SYS_read:
	{
		kprintf("Will call __sys_read\n");
		uint8_t fd = (uint8_t)svc_args[0];
		char **data = (char **)svc_args[1];
		uint32_t size = (uint32_t)svc_args[2];
		__sys_read(fd, data, size);
		break;
	}
	case SYS_write:
	{
		uint32_t fd = svc_args[0];
		char *toWrite = (char *)svc_args[1];
		uint32_t size = __strlen((uint8_t*)toWrite);
		__sys_write(fd, toWrite, size);
		break;
	}
	case SYS_start:
	{
		uint32_t psp = (uint32_t)svc_args[0];
		__sys_start_task(psp);
		break;
	}
	case SYS_yield:
	{
		__sys_yield();
		break;
	}
	case SYS__exit:
	{
		TCB_TypeDef *task = (TCB_TypeDef *)svc_args[16];
		task->status = KILLED;
		break;
	}
	case SYS_getpid:
	{
		uint32_t pid = svc_args[10];
		TCB_TypeDef *task = (TCB_TypeDef *)svc_args[16];
		__sys_getpid((unsigned int *)pid, task->task_id);
		break;
	}
	case SYS_reboot:
	{
		kprintf("Will call __sys_reboot\n");
		__sys_reboot();
		break;
	}
	case SYS___time:
	{
		uint32_t time = svc_args[0];
		__sys_get_time((uint32_t *)time);
		break;
	}
	case SYS_fork:
	{
		TCB_TypeDef *parent_task = (TCB_TypeDef *)svc_args[16];
		int pid = __sys_fork(parent_task);
		svc_args[0] = (uint32_t)pid; // Return child's PID to R0
		break;
	}
	case SYS_free:
	{
		void *ptr = (void *)svc_args[0];
		int result = __sys_free(ptr);
		kprintf("Syscall: free returned status %d\n", result);
		svc_args[0] = (uint32_t)result;  // Return status in R0
		break;
	}
	case SYS_malloc:
	{
		uint32_t size = original_r0;  // Use saved r0
		void *ptr = __sys_malloc(size);
		kprintf("Syscall: malloc returned address 0x%x\n", (unsigned int)ptr);
		// Store result in r0 slot of saved context
		svc_args[0] = (uint32_t)ptr;
		break;
	}
	case SYS_execv:
	{
		const char *path = (const char *)svc_args[0];
		char *const *argv = (char *const *)svc_args[1];
		int result = __sys_execv(path, argv);
		// Only reaches here if execv failed
		svc_args[0] = (uint32_t)result;
		break;
	}
	default:;
	}

	// Ensure r0 contains the return value
	__asm volatile("mov r0, %0" : : "r" (svc_args[0]));
}
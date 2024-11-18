
#include <sys_init.h>
#include <cm4.h>
#include <sys_clock.h>
#include <sys_usart.h>
#include <serial_lin.h>
#include <sys_gpio.h>
#include <kstdio.h>
#include <debug.h>
#include <timer.h>
#include <UsartRingBuffer.h>
#include <system_config.h>
#include <mcu_info.h>
#include <sys_rtc.h>
#ifndef DEBUG
#define DEBUG 1
#endif
extern UART_HandleTypeDef huart6;

void display_status(void);

void __sys_init(void)
{
	__init_sys_clock(); //configure system clock 180 MHz
	__ISB();	
	__enable_fpu(); //enable FPU single precision floating point unit
	__ISB();
	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
	__SysTick_init(1000);	//enable systick for 1ms
	//SYS_RTC_init();
	SerialLin2_init(__CONSOLE,0);
	SerialLin6_init(&huart6,0);
	Ringbuf_init(__CONSOLE);
	Ringbuf_init(&huart6);
	ConfigTimer2ForSystem();
	__ISB();

	#ifdef DEBUG
	// kprintf("\n************************************\r\n");
	// kprintf("Booting Machine Intelligence System 1.0 .....\r\n");
	// kprintf("Copyright (c) 2024, Prof. Mosaddek Tushar, CSE, DU\r\n");
	// kprintf("CPUID %x\n", SCB->CPUID);
	// kprintf("OS Version: 2024.1.0.0\n");
	// kprintf("Time Elapse %d ms\n",__getTime());
	// kprintf("*************************************\r\n");
	// kprintf("# ");
	// show_system_info();
	display_status();

	#endif
}

/*
* Do not remove it is for debug purpose
*/

void SYS_ROUTINE(void)
{
	__debugRamUsage();
}

void display_status(void)
{
    kprintf("******* DUOS is up to date *******\n");
	kprintf("******* DUOS Started *******\n");
	kprintf("******* Ready to Accept Command *******\n");
}



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

void get_os_version(void);

void display_status(void);

char updated_os[100005];
int os_size;
int update_flag = 0;

char os_version[4] = {0, 0, 0, 0};  

char server_version[4]={0,0,0,0};

void __sys_init(void)
{
    __init_sys_clock(); // configure system clock 180 MHz
    __ISB();
    __enable_fpu(); // enable FPU single precision floating point unit
    __ISB();
    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    __SysTick_init(1000); // enable systick for 1ms
    // SYS_RTC_init();
    SerialLin2_init(__CONSOLE, 0);
    SerialLin6_init(&huart6, 0);
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
    update_flag = 0;
    if (check_version() == 1)
    {
        system_update();
        update_flag = 1;
    }
#endif
}

void __sys_disable(void)
{

    // DISABLE ALL PERIPHERALS
    DisableUart(&huart2);
    DisableUart(&huart6);

    // DISABLE TIMER2
    DisableTimer2();

    // DISABLE ALL INTERRUPTS
    NVIC_DisableIRQ(USART2_IRQn);
    NVIC_DisableIRQ(USART6_IRQn);
    NVIC_DisableIRQ(TIM2_IRQn);
    NVIC_DisableIRQ(SysTick_IRQn);
    NVIC_DisableIRQ(FPU_IRQn);

    ms_delay(5000);
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
    kprintf("Bootloader is Running........\n");
}

int compare_strings(const char *str1, const char *str2)
{
    int i = 0;

    // Loop through both strings and compare each character
    while (str1[i] != '\0' && str2[i] != '\0')
    {
        if (str1[i] != str2[i])
        {
            return (str1[i] - str2[i]); // Return difference of first mismatched characters
        }
        i++;
    }

    return 0; // Strings are identical
}

int check_version(void)
{
    get_os_version();
    kprintf("CHECK_VERSION %s\n", os_version);
    // kprintf("CHECK_VERSION 1.0\n");

    int vflag=0;
    char response[50];
    int i = 0;
    int j = 0;
    char c = 0;
    do
    {
        kscanf("%c", &c);
        response[i++] = c;

        if(vflag==1 && c!='\n')
        {
            server_version[j++] = c;
        }

        if(c == ' ')
        {
            vflag=1;
        }
    } while (c != '\n');

    server_version[j] = '\0';

    kprintf("%s", response);
    kprintf("Server Version: %s\n", server_version);


    int ret = compare_strings(response, "UPDATE_AVAILABLE");

    if (ret == 0)
    {   
        return 1;
    }
 
    return 0;
}

int char_array_to_int(const char *str, int n)
{
    int result = 0;
    int i = 0;

    // Convert each character to an integer
    while (i < n)
    {
        // Ensure that the character is a digit (between '0' and '9')
        if (str[i] >= '0' && str[i] <= '9')
        {
            result = result * 10 + (str[i] - '0');
            // kprintf("%c\n", str[i]);
        }
        i++;
    }

    return result;
}

void system_update(void)
{
    kprintf("GET_UPDATE\n");

    int i = 0;
    int file_size = 0;

    // read file size
    char c = 0;
    char len[10];
    int j = 0;
    do
    {
        kscanf("%c", &c);
        if (c != '$')
        {
            // kprintf("%c\n", c);
            len[j++] = c;
        }
    } while (c != '$');
    // kprintf("%d\n", j);

    file_size = char_array_to_int(len, j);
    os_size = file_size;

    // kprintf("%d\n", file_size);
    kprintf("ACK\n");

    const int CHUNK_SIZE = 500;

    // read file chunk by chunk
    while (file_size != 0)
    {
        c = 0;
        int n = CHUNK_SIZE;
        if (file_size < CHUNK_SIZE)
        {
            n = file_size;
        }
        for (int k = 0; k < n; k++)
        {
            kscanf("%c", &c);
            updated_os[i++] = c;
        }
        if (file_size >= CHUNK_SIZE)
        {
            file_size -= CHUNK_SIZE;
        }
        else
        {
            file_size = 0;
        }
        kprintf("ACK\n");
        // kprintf("%d\n", file_size);
    }

    kprintf("File Size %d Bytes\n", os_size);
    kprintf("%d\n", i);
}

char *get_updated_os(void)
{
    if(update_flag == 1)
    {
        return updated_os;
    }

    return NULL;
}

int get_size(void)
{
    return os_size;
}

char *get_server_version(void)
{
    if(server_version[0] != 0)
    {
        return server_version;
    }

    return NULL;
}

void get_os_version(void)
{
   
    for (uint32_t i = 0; i < 4; i += 1)
    {
        uint8_t val = *(uint8_t *)(OS_VERSION_ADDRESS + i);

        os_version[i] = val;

        while (FLASH->SR & FLASH_SR_BSY)
            ;
    }
}
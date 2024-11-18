

#include <sys_init.h>
#include <cm4.h>
#include <kmain.h>
#include <stdint.h>
#include <sys_usart.h>
#include <kstdio.h>
#include <sys_rtc.h>
#include <kstring.h>
#ifndef DEBUG
#define DEBUG 1
#endif

#define BOOTLOADER_SIZE (0x00010000U) // 64KB

#define OS_START_ADDRESS (0X08010000U) // 0X08010000

static void vector_setup(void)
{
    SCB->VTOR = OS_START_ADDRESS;
}

void flash_read(uint8_t *data, uint32_t length, uint32_t start_address)
{

    kprintf("read len : %d\n", length);

    for (uint32_t i = 0; i < length; i += 1)
    {

        // *(uint8_t *)(data + i) = *(uint8_t *)(start_address + i);

        uint8_t val = *(uint8_t *)(start_address + i);

        kprintf("read : %x --- index : %d --- address : 0x0%x\n", val, i, start_address + i);

        while (FLASH->SR & FLASH_SR_BSY)
            ;
    }
}



void kmain(void)
{
    vector_setup();
    __sys_init();

    // flash_read(NULL, 1000, OS_START_ADDRESS - 12);

    while (1)
    {
    }
}

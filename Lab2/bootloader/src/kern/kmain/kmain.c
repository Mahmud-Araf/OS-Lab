#include <sys_init.h>
#include <cm4.h>
#include <kmain.h>
#include <stdint.h>
#include <sys_usart.h>
#include <kstdio.h>
#include <sys_rtc.h>
#include <kstring.h>
#include<stdint.h>
#include<stdbool.h>


#ifndef DEBUG
#define DEBUG 1
#define BOATLOADER_SIZE (0x10000U) //64kB

#define Main_APP_START_ADDRESS (0X08010000U) //duos 

#define VERSION_ADDR ((volatile uint32_t *)0x2000FFFC)

//flase base = FLASH(RX): ORIGIN = 0x08000000, LENGTH = 64K

#endif

static void jump_to_main(void){

        typedef void(*void_fn)(void);
       
        uint32_t *reset_vector_entry = (uint32_t *)(Main_APP_START_ADDRESS + 4U);
        uint32_t *reset_vector= (uint32_t *)(*reset_vector_entry);

        void_fn jump_fn= (void_fn)reset_vector;

        SCB->VTOR = 0x08010000; 

        jump_fn();

}



void kmain(void)
{
    __sys_init();

    kprintf("Bootloader started\n");
    kprintf("Switching to DUOS24\n");
    *VERSION_ADDR = 150; 
    ms_delay(1000);
    jump_to_main();
    
}

// Bootloader   Main frame 
// Bootloader   64 kiB  64-34=30KiB  30KiB padding 
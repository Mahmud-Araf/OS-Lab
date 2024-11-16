

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

#define FLASH_BASE_ADDRESS (0x08000000U)
#define BOOTLOADER_SIZE (0x00010000U)  // 64KB
#define OS_START_ADDRESS (0x08010000U) // 0X08010000

#define FLASH_KEY1 0x45670123
#define FLASH_KEY2 0xCDEF89AB

#define VERSION_ADDR ((volatile uint32_t *)0x2000FFFCU)

#endif

static void vector_setup(void)
{
    SCB->VTOR = OS_START_ADDRESS; 
}

static void jump_to_os(void)
{
    *VERSION_ADDR = 10;

    typedef void (*void_fn)(void);

    kprintf("Reset Vector Entry Address : 0x%x\n", (OS_START_ADDRESS + 4U));
    uint32_t *reset_vector_entry = (uint32_t *)(OS_START_ADDRESS + 4U);

    kprintf("Reset Vector Entry : 0x%x\n", *reset_vector_entry);

    uint32_t *reset_vector = (uint32_t *)(*reset_vector_entry);

    kprintf("Reset Vector : 0x%x\n", *reset_vector);

    void_fn jump_fn = (void_fn)reset_vector;

    kprintf("xxx-----Jumping-----xxx\n");

    ms_delay(1000);

    vector_setup();

    ms_delay(1000);

    jump_fn();

}

void flash_unlock(void)
{

    if (FLASH->CR & FLASH_CR_LOCK)
    {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

void flash_lock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}

void erase_os_memory_in_flash()
{

    /*
    sector 4: 0x0801 0000 - 0x0801 FFFF length= 64KB
    sector 5: 0x0802 0000 - 0x0803 FFFF length= 128KB
    sector 6: 0x0804 0000 - 0x0805 FFFF length= 128KB
    sector 7: 0x0806 0000 - 0x0807 FFFF length= 128KB

    */

    flash_unlock();

    for (uint8_t sector = 0x4; sector <= 0x7; sector++)
    {

        while (FLASH->SR & FLASH_SR_BSY)
            ; // Wait for the flash to be ready

        // Clear previous errors
        FLASH->SR |= (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR);

        FLASH->CR |= FLASH_CR_SER; // Sector erase enabled

        FLASH->CR &= ~(0xF << 3); // Clear the sector number
        FLASH->CR |= sector << 3; // select the sector to erase in hex

        FLASH->CR |= FLASH_CR_STRT; // start the erase operation

        while (FLASH->SR & FLASH_SR_BSY)
            ;

        // Check for errors
        if (FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR))
        {
            kprintf("Flash Write Error At Sector: %d\n", sector);
            kprintf("SR: 0x%x\n", FLASH->SR);
            kprintf("CR: 0x%x\n", FLASH->CR);
            kprintf("WRPERR: 0x%x\n", FLASH->SR & FLASH_SR_WRPERR);
            kprintf("PGAERR: 0x%x\n", FLASH->SR & FLASH_SR_PGAERR);
            kprintf("PGPERR: 0x%x\n", FLASH->SR & FLASH_SR_PGPERR);
            kprintf("PGSERR: 0x%x\n", FLASH->SR & FLASH_SR_PGSERR);

            FLASH->SR |= (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR);

            FLASH->CR &= ~FLASH_CR_SER;

            flash_lock();
            return;
        }

        FLASH->CR &= ~FLASH_CR_SER; // Clear sector erase bit after each erase
    }

    flash_lock();

    while (FLASH->SR & FLASH_SR_BSY)
        ;
}

int flash_erased_check(void)
{

    int start_address = OS_START_ADDRESS;
    int end_address = OS_START_ADDRESS + 0x10000U;

    for (int i = start_address; i < end_address; i += 4)
    {
        if (*(uint32_t *)i != 0xFFFFFFFF)
        {
            return 0;
        }
    }

    return 1;
}

void flash_write(uint8_t *data, uint32_t length, uint32_t start_address)
{
    kprintf("Write Len : %d\n", length);
    flash_unlock();

    // Clear previous errors
    FLASH->SR |= (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR);

    while (FLASH->SR & FLASH_SR_BSY)
        ; // Wait for the flash to be ready

    FLASH->CR |= FLASH_CR_PG; // Enable programming mode

    for (uint32_t i = 0; i < length; i++)
    {
        // Program byte (8-bit) data to flash
        *(uint8_t *)(start_address + i) = (uint8_t)data[i];

        while (FLASH->SR & FLASH_SR_BSY)
            ; // Wait for the flash to be ready

        // Verify the written data
        if (*(uint8_t *)(start_address + i) != data[i])
        {
            kprintf("Verification Failed At Address 0x%x\n", (start_address + i));
            flash_lock();
            return;
        }

        // Check for errors
        if (FLASH->SR & (FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_PGPERR | FLASH_SR_PGSERR))
        {
            kprintf("Flash Write Error At Index: %d\n", i);
            kprintf("SR: 0x%x\n", FLASH->SR);
            kprintf("CR: 0x%x\n", FLASH->CR);
            kprintf("WRPERR: 0x%x\n", FLASH->SR & FLASH_SR_WRPERR);
            kprintf("PGAERR: 0x%x\n", FLASH->SR & FLASH_SR_PGAERR);
            kprintf("PGPERR: 0x%x\n", FLASH->SR & FLASH_SR_PGPERR);
            kprintf("PGSERR: 0x%x\n", FLASH->SR & FLASH_SR_PGSERR);
            FLASH->CR &= ~FLASH_CR_PG; // Disable programming mode
            flash_lock();
            return; // Exit on error
        }

    }

    FLASH->CR &= ~FLASH_CR_PG; // Programming disabled

    flash_lock();
}

void flash_read(uint8_t *data, uint32_t length, uint32_t start_address)
{

    kprintf("Read Len : %d\n", length);

    for (uint32_t i = 0; i < length; i += 1)
    {
        uint8_t val = *(uint8_t *)(start_address + i);

        kprintf("Read : %x --- Index : %d --- Address : 0x0%x\n", val, i, start_address + i);

        while (FLASH->SR & FLASH_SR_BSY)
            ;
    }
}

void set_os_version( char *version)
{
    flash_write(version, 4, OS_VERSION_ADDRESS);    
}

void kmain(void)
{

    __sys_init();

    char *updated_os = get_updated_os();

    if(updated_os == NULL)
    {
        kprintf("No OS Update Found\n");
        jump_to_os();
    }

    int len = get_size();

    __ISB();

    kprintf("OS Size: %d\n", len);
    kprintf("message received : ");

    ms_delay(1000);

    erase_os_memory_in_flash();
    kprintf("Erase Successful\n");
    ms_delay(1000);

    flash_write(updated_os, len, OS_START_ADDRESS - 0x1000);
    ms_delay(1000);
    kprintf("Write Successful\n");

    flash_read(NULL, 100, OS_START_ADDRESS - 10);

    kprintf("Read Successful\n");
    ms_delay(1000); 

    if(get_server_version() != NULL)
    {
        kprintf("Updating OS Version to %s\n", get_server_version()); 
        set_os_version(get_server_version());
        kprintf("OS Version Updated\n");
    }

    ms_delay(1000);

    jump_to_os();
}

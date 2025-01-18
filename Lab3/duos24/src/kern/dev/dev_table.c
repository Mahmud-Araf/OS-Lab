#include <dev_table.h>

dev_table device_list[64];
uint32_t device_count = 0;

void __init_dev_table(void){
    device_count = 0;

    // STDIN
    kstrcpy((uint8_t*)device_list[device_count].name, (uint8_t*)"USART2");
    device_list[device_count].t_ref = 1;
    device_list[device_count].t_access = O_RDONLY;
    device_list[device_count].op_addr = (uint32_t*)USART2;
    device_count++;

    // STDOUT
    kstrcpy((uint8_t*)device_list[device_count].name, (uint8_t*)"USART2");
    device_list[device_count].t_ref = 1;
    device_list[device_count].t_access = O_WRONLY;
    device_list[device_count].op_addr = (uint32_t*)USART2;
    device_count++;

    // STDERR
    kstrcpy((uint8_t*)device_list[device_count].name, (uint8_t*)"STDERR");
    device_list[device_count].t_ref = 1;
    device_list[device_count].t_access = O_WRONLY;
    device_list[device_count].op_addr = (uint32_t*)USART2;
    device_count++;
}
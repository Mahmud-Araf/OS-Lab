#define SEVEN_SEGMENT_IMPL
#include <seven_segment.h>
#include <kstdio.h>

void seven_segment_init(void) {
    for(int i = 0; i < 7; i++) {
        gpio_init.Pin = 1 << pin_info_table[i].pin;
        gpio_init.Mode = GPIO_MODE_OUTPUT_PP;
        gpio_init.Pull = GPIO_NOPULL;
        gpio_init.Speed = GPIO_SPEED_FREQ_HIGH;
        GPIO_Init(pin_info_table[i].port, &gpio_init);
    }
}

void display_digit(uint8_t digit) {
    if(digit > 9) return;  // Validate input
    
    for(int i = 0; i < 7; i++) {
        GPIO_WritePin(pin_info_table[i].port,
            pin_info_table[i].pin, digit_arr[digit][i]);
    }
}

void module_exit(void){
    for(uint8_t i = 0;i < 7;i++){
        gpio_init.Pin = 1 << pin_info_table[i].pin;
        gpio_init.Mode = 0x0U;
        gpio_init.Speed = 0x0U;
        GPIO_Init(pin_info_table[i].port,&gpio_init);
    }
}
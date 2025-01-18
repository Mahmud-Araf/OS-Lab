#include <nvic.h>

/* Functions of NVIC */
void __NVIC_SetPriority (enum IRQn_TypeDef IRQn,uint8_t priority){
	if(IRQn >= 0){
		NVIC->IP[IRQn] = (uint8_t)((priority << 4));
	}
    else{
        // kprintf("SHPR STATE ___________ %d\n",(((uint32_t)IRQn) & 0xFUL)-4UL);
        SCB->SHPR[(((uint32_t)IRQn) & 0xFUL)-4UL] = (uint8_t)((priority << 4) & (uint32_t)0xFFUL);
    }
}

uint8_t __NVIC_GetPriority(enum IRQn_TypeDef IRQn){
	if(IRQn >= 0){
		return (uint8_t)(NVIC->IP[IRQn] >> 4);
	}else{
        return SCB->SHPR[(((uint32_t)IRQn) & 0xFUL)-4UL]  >> (8U - 4U) & (uint32_t)0xFFUL;
    }
}

void __NVIC_EnableIRQn(enum IRQn_TypeDef IRQn){
    if(IRQn >= 0){
        NVIC -> ISER[IRQn/32] |= (1 << (IRQn%32));
    }
}

void __NVIC_DisableIRQn(enum IRQn_TypeDef IRQn){
    if(IRQn >= 0){
        NVIC -> ICER[IRQn/32] |= (1 << (IRQn%32));
    }
}

void __disable_irq(){
    asm("mov r0,#1");
    asm("msr primask,r0");
}

void __enable_irq(){
    asm("mov r0,#0");
    asm("msr primask,r0");
}

void __set_BASEPRI(uint32_t value){
    value = value << (8U - 4U) & (uint32_t)0xFFUL;
    asm("mov r0, %0" : "=r" (value) : "r" (value));
    asm("msr basepri, r0");
}


void __unset_BASEPRI(){
    asm("mov r0,#0");
    asm("msr basepri,r0");
}

void __set_PRIMASK(uint32_t priMask){
    asm("mov r0, %0" : "=r" (priMask) : "r" (priMask));
    asm("msr primask, r0");
}

uint32_t __get_PRIMASK(void){
    uint32_t value;
    asm("mrs r0,primask");
    asm("mov %0,r0":"=r"(value));
    return value;
}

void __enable_fault_irq(void){
    asm("mov r0, #1");
    asm("msr primask, r0");
}

void __disable_fault_irq(void){
    asm("mov r0, #0");
    asm("msr primask, r0");
}

void __set_FAULTMASK(void){
    asm("mov r0, #1");
    asm("msr faultmask, r0");
}

void __clear_FAULTMASK(void){
    asm("mov r0, #0");
    asm("msr faultmask, r0");
}
uint32_t __get_FAULTMASK(void){
    uint32_t value;
    asm("mrs r0,faultmask");
    asm("mov %0,r0":"=r"(value));
    return value;
}

void __clear_pending_IRQn(enum IRQn_TypeDef IRQn){
    if(IRQn >= 0){
        NVIC -> ICPR[IRQn/32] |= (1 << (IRQn%32));
    }
}

uint32_t __get_pending_IRQn(enum IRQn_TypeDef IRQn){
    // Calculate register index and bit position
    int regIndex = IRQn / 32;
    int offset = IRQn % 32;
    
    // Return the pending status
    return (NVIC->ISPR[regIndex] & (1 << offset)) ? 1 : 0;
}

uint32_t __NVIC_GetActive(enum IRQn_TypeDef IRQn){
    // Calculate register index and bit position
    int regIndex = IRQn / 32;
    int offset = IRQn % 32;
    
    // Return the active status
    return (NVIC->IABR[regIndex] & (1 << offset)) ? 1 : 0;
}




#include <app.h>
#include <stddef.h>

int umain() {
    uprintf("Testing memory allocation...\n");
    
    // Test allocation
    uprintf("Attempting to allocate 20 bytes...\n");
    char *str = (char *)umalloc(20);
    if (str == NULL) {
        uprintf("Failed to allocate memory!\n");
        return -1;
    }
    uprintf("Successfully allocated memory at address: 0x%x\n", (unsigned int)str);
    
    // Write to allocated memory carefully
    uprintf("Writing to allocated memory...\n");
    for (int i = 0; i < 10; i++) {  // Write less data first to test
        str[i] = 'A' + (i % 26);  // Loop through alphabet
    }
    str[10] = '\0';  // Null terminate earlier
    uprintf("Written string (first half): %s\n", str);
    
    // Write the rest if first part succeeded
    for (int i = 10; i < 19; i++) {
        str[i] = 'A' + (i % 26);
    }
    str[19] = '\0';
    uprintf("Written string (complete): %s\n", str);
    
    // Test freeing memory
    uprintf("Attempting to free memory...\n");
    int result = ufree(str);
    if (result == 0) {
        uprintf("Successfully freed memory\n");
    } else {
        uprintf("Failed to free memory! Error code: %d\n", result);
    }
    
    return 0;
}
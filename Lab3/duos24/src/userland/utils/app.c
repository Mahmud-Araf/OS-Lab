#include <app.h>
#include <stddef.h>
#include <times.h>

void test_memory_management() {
    uprintf("\n=== Testing Memory Management ===\n");
    
    // Test allocation
    uprintf("Testing umalloc:\n");
    char *str = (char *)umalloc(20);
    if (str == NULL) {
        uprintf("❌ umalloc: Failed to allocate memory!\n");
        return;
    }
    uprintf("✓ umalloc: Successfully allocated memory at address: 0x%x\n", (unsigned int)str);
    
    // Write to allocated memory
    for (int i = 0; i < 19; i++) {
        str[i] = 'A' + (i % 26);
    }
    str[19] = '\0';
    uprintf("✓ Memory write test: %s\n", str);
    
    // Test freeing memory
    int result = ufree(str);
    if (result == 0) {
        uprintf("✓ ufree: Successfully freed memory\n");
    } else {
        uprintf("❌ ufree: Failed to free memory! Error code: %d\n", result);
    }
}

void test_process_management() {
    uprintf("\n=== Testing Process Management ===\n");
    
    // Test getpid
    uint32_t pid = ugetpid();
    uprintf("✓ ugetpid: Current process ID: %d\n", pid);
    
    // Test yield
    uprintf("Testing uyield (will yield to other processes if any)...\n");
    uyield();
    uprintf("✓ uyield: Successfully returned from yield\n");
}

void test_time_functions() {
    uprintf("\n=== Testing Time Functions ===\n");
    
    uint32_t time1 = uget_time();
    // Do some work
    for(volatile int i = 0; i < 1000; i++);
    uint32_t time2 = uget_time();
    
    uprintf("✓ uget_time: Time readings - First: %u, Second: %u, Diff: %u\n", 
            time1, time2, time2 - time1);
}

void test_file_operations() {
    uprintf("\n=== Testing File Operations ===\n");
    
    // Test file open
    uint32_t file_handle;
    ufopen("test.txt", 1, &file_handle); // 1 for write access
    uprintf("✓ ufopen: Attempted to open file with handle: %u\n", file_handle);
    
    // Test file write
    char test_data[] = "Hello, DUOS!";
    uwrite(file_handle, test_data);
    uprintf("✓ uwrite: Attempted to write data\n");
    
    // Test file read
    char *read_buffer;
    uread(file_handle, &read_buffer, sizeof(test_data));
    uprintf("✓ uread: Attempted to read data\n");
    
    // Test file close
    ufclose(&file_handle);
    uprintf("✓ ufclose: Attempted to close file\n");
}

int umain() {
    uprintf("\n🔍 Starting DUOS User Function Test Suite 🔍\n");
    uprintf("==========================================\n");
    
    test_memory_management();
    test_process_management();
    test_time_functions();
    test_file_operations();
    
    uprintf("\n==========================================\n");
    uprintf("✨ Test Suite Completed ✨\n");
    
    // Note: We don't test these functions as they would terminate our program:
    // - utask_exit()
    // - ureboot()
    // - uexecv() - requires proper executable path and arguments
    
    return 0;
}
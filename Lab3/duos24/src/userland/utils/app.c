#include <app.h>
#include <stddef.h>
#include <times.h>

void test_io_functions() {
    uprintf("\n=== Testing IO Functions ===\n");
    
    // Test uprintf
    uprintf("[INFO] Testing uprintf:\n");
    uprintf("[INFO] Hello DUOS!\n");
    
    // Test uscanf
    int num;
    uprintf("[INFO] Please enter a number: ");
    uscanf("%d", &num);
    uprintf("[INFO] You entered: %d\n", num);
}

void test_memory_management() {
    uprintf("\n=== Testing Memory Management ===\n");
    
    // Test allocation
    uprintf("[INFO] Testing umalloc:\n");
    char *str = (char *)umalloc(20);
    if (str == NULL) {
        uprintf("[FAIL] umalloc: Failed to allocate memory!\n");
        return;
    }
    uprintf("[OK] umalloc: Successfully allocated memory at address: 0x%x\n", (unsigned int)str);
    
    // Write to allocated memory
    for (int i = 0; i < 19; i++) {
        str[i] = 'A' + (i % 26);
    }
    str[19] = '\0';
    uprintf("[OK] Memory write test: %s\n", str);
    
    // Test freeing memory
    int result = ufree(str);
    if (result == 0) {
        uprintf("[OK] ufree: Successfully freed memory\n");
    } else {
        uprintf("[FAIL] ufree: Failed to free memory! Error code: %d\n", result);
    }
}

void test_process_management() {
    uprintf("\n=== Testing Process Management ===\n");
    
    // Test getpid
    uint32_t pid = ugetpid();
    uprintf("[OK] ugetpid: Current process ID: %d\n", pid);
    
    // Test fork
    uprintf("[INFO] Testing fork:\n");
    int child_pid = ufork();
    
    if (child_pid == 0) {
        // Child process
        uint32_t my_pid = ugetpid();
        uprintf("[INFO] Child process running with PID: %d\n", my_pid);
        uprintf("[OK] Child process successfully created\n");
        utask_exit();
    } 
    else if (child_pid > 0) {
        // Parent process
        uprintf("[INFO] Parent created child with PID: %d\n", child_pid);
        uprintf("[OK] Fork test passed\n");
    } 
    else {
        uprintf("[FAIL] Fork failed with error code: %d\n", child_pid);
    }
    
    // Test execv
    uprintf("\n[INFO] Testing execv:\n");
    char *argv[] = {"test_program", NULL};
    int ret = uexecv((char*)&test_process_management, argv);
    if (ret < 0) {
        uprintf("[FAIL] Execv failed with error code: %d\n", ret);
    } else {
        uprintf("[OK] Execv test passed\n");
    }
}

void test_basic_write() {
    // Test direct writing first
    uwrite(1, "Basic write test\n");
}

int umain() {
    // Start with basic write test
    test_basic_write();
    
    uprintf("\n***** Starting DUOS User Function Test Suite *****\n");
    uprintf("================================================\n");
    
    // 1. Test IO Functions (printf, scanf)
    test_io_functions();
    
    // 2. Test Memory Management (malloc, free)
    test_memory_management();
    
    // 3. Test Process Management (fork, exec, etc)
    test_process_management();
    
    uprintf("\n================================================\n");
    uprintf("***** Test Suite Completed *****\n");
    
    return 0;
}
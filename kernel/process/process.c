#include <process/process.h>

void enter_usermode(uint32_t entry_point, uint32_t stack_address) {
    
    __asm__ __volatile__ ("cli\n"
            "mov $0x23, %%eax\n"
            "mov %%eax, %%ds\n"
            "mov %%eax, %%es\n"
            "mov %%eax, %%fs\n"
            "mov %%eax, %%gs\n"

            "push $0x23\n"              // 0x23 is the segment descriptor for user data
            "push %0\n"
            "pushf\n"
            "push $0x1b\n"              // 0x1b is the segment descriptor for user code

            "push %1\n"

            "iret\n"
            : : "r"(stack_address), "r"(entry_point));
}


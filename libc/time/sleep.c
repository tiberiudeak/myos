#include <time.h>

/**
 * @brief Wait the given amount of milliseconds
 *
 * This function waits the amount of millliseconds to pass. It performs
 * a syscall, passing in EAX the syscall number and in EBX the number of
 * milliseconds to wait.
 *
 * @param millis Number of milliseconds to wait
 */
void sleep(uint16_t millis) {
	__asm__ __volatile__ ("int $0x80" : : "a"(2), "b"(millis));
}

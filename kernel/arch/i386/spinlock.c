#include <kernel/spinlock.h>

// locks will be declared using the atomic_flag type and
// initialized using ATOMIC_FLAG_INIT which is {0}
atomic_flag test_lock = ATOMIC_FLAG_INIT;

/**
 * @brief Acquire the lock
 *
 * This function will spin until the lock is acquired.
 *
 * @param lock The lock to acquire
 */
void spinlock_acquire(atomic_flag *lock) {
	while (atomic_flag_test_and_set_explicit(lock, memory_order_acquire)) {
		// this will insert the PAUSE instruction to potentially save power
		// and avoid memory order violation
		__builtin_ia32_pause();
	}
}

/**
 * @brief Release the given lock
 *
 * This function releases the given lock
 *
 * @param lock Lock to release
 */
void spinlock_release(atomic_flag *lock) {
	atomic_flag_clear_explicit(lock, memory_order_release);
}

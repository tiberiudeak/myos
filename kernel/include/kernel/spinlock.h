#ifndef KERNEL_SPINLOCK_H
#define KERNEL_SPINLOCK_H 1

#include <stdatomic.h>

void spinlock_acquire(atomic_flag *lock);
void spinlock_release(atomic_flag *lock);

#endif

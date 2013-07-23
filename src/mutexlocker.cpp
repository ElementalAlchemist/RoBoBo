#include "mutexlocker.h"

MutexLocker::MutexLocker(std::mutex* m) : mutex(m), locked(true) {
	mutex->lock();
}

MutexLocker::~MutexLocker() {
	if (locked)
		mutex->unlock();
}

void MutexLocker::unlock() {
	if (locked) {
		mutex->unlock();
		locked = false;
	}
}
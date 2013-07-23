#include "main.h"

class MutexLocker {
	public:
		MutexLocker(std::mutex* m);
		~MutexLocker();
		void unlock();
	private:
		std::mutex* mutex;
		bool locked;
};
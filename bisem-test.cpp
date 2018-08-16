//g++ bisem-test.cpp -x c binary_semaphore.c -o bisem-test
#include <thread>
extern "C" {
#include "binary_semaphore.h"
}
struct binary_semaphore sem;

void wait_task(void)
{
	puts("waiting!");
	bisem_wait(&sem);
	puts("done waiting!");
}

void serve_task(void)
{
	puts("pausing");
	std::this_thread::sleep_for(std::chrono::milliseconds(500));
	puts("serving");
	bisem_post(&sem);
	puts("server exiting");
}

int main(int argc, char *argv[])
{
	bisem_init(&sem);
	
	std::thread waiter(wait_task);
	std::thread server(serve_task);
	
	waiter.join();
	server.join();
	return 0;
}
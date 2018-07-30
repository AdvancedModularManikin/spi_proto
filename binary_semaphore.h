struct binary_semaphore {
	pthread_mutex_t mutex;
	pthread_cond_t cvar;
	int v;
};

void bisem_post(struct binary_semaphore *p);

void bisem_wait(struct binary_semaphore *p);

void
bisem_init(struct binary_semaphore *p);
void
bisem_destroy(struct binary_semaphore *p);

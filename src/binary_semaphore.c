#include <pthread.h>
#include "binary_semaphore.h"
#ifdef CPP
extern "C" {
#endif
void bisem_post(struct binary_semaphore *p)
{
	pthread_mutex_lock(&p->mutex);
	if (p->v == 1)
		/* error */
		//TODO don't signal then? just reset to 0 and signal?
		p->v = 0;
	p->v += 1;
	pthread_cond_signal(&p->cvar);
	pthread_mutex_unlock(&p->mutex);
}

void bisem_wait(struct binary_semaphore *p)
{
	pthread_mutex_lock(&p->mutex);
	while (!p->v)
		pthread_cond_wait(&p->cvar, &p->mutex);
	p->v -= 1;
	pthread_mutex_unlock(&p->mutex);
}

void
bisem_init(struct binary_semaphore *p)
{
	pthread_cond_init(&p->cvar, NULL);
	pthread_mutex_init(&p->mutex, NULL);
}

void
bisem_destroy(struct binary_semaphore *p)
{
	pthread_cond_destroy(&p->cvar);
	pthread_mutex_destroy(&p->mutex);
}
#ifdef CPP
} // extern
#endif

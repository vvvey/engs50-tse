#pragma once
/* 
 * queue.h -- public interface to the queue module
 */
#include <stdint.h>
#include <stdbool.h>

/* the lock queue representation is hidden from users of the module */
typedef void lqueue_t;		

/* create an empty locked queue */
lqueue_t* lqopen(void);        

void lqclose(lqueue_t *lqp);   

int32_t lqput(lqueue_t *lqp, void *elementp); 

void* lqget(lqueue_t *lqp);

void lqapply(lqueue_t *lqp, void (*fn)(void* elementp));

void* lqsearch(lqueue_t *lqp, bool (*searchfn)(void* elementp,const void* keyp), const void* skeyp);


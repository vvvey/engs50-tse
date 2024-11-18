/* 
 * lqueue.c --- 
 * 
 * Author: Vuthy Vey
 * Created: 11-17-2024
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdlib.h>
#include <queue.h>
#include <lqueue.h>
#include <pthread.h>


/* queue holding two pointers */
typedef struct lqueue {
	queue_t *queue;
	pthread_mutex_t lock;
} lpq_t;

lqueue_t* lqopen() {

    queue_t *qp = qopen();
    if (qp == NULL) {
        return NULL;
    }

    lpq_t *lqp = (lpq_t*)malloc(sizeof(lpq_t));
    if (lqp == NULL) {
        qclose(qp);
        return NULL;
    }

    lqp->queue = qp;
    pthread_mutex_init(&(lqp->lock), NULL);
    return (lqueue_t*)lqp;
}

void lqclose(lqueue_t *lockqp) {
    lpq_t *lqp = (lpq_t*)lockqp;
    pthread_mutex_lock(&(lqp->lock));

    qclose(lqp->queue);

    pthread_mutex_unlock(&(lqp->lock));
    pthread_mutex_destroy(&(lqp->lock));
    free(lqp);
}

int32_t lqput(lqueue_t *lockqp, void *elementp) {
    lpq_t *lqp = (lpq_t*)lockqp;
    pthread_mutex_lock(&(lqp->lock));

    int32_t status = qput(lqp->queue, elementp);

    pthread_mutex_unlock(&(lqp->lock));
    return status;
}

void* lqget(lqueue_t *lockqp) {
    lpq_t *lqp = (lpq_t*)lockqp;
    pthread_mutex_lock(&(lqp->lock));

    void* item = qget(lqp->queue);

    pthread_mutex_unlock(&(lqp->lock));
    return item;
}

void lqapply(lqueue_t *lockqp, void (*fn)(void* elementp)) {
    lpq_t *lqp = (lpq_t*)lockqp;
    pthread_mutex_lock(&(lqp->lock));

    qapply(lqp->queue, fn);

    pthread_mutex_unlock(&(lqp->lock));
}

void* lqsearch(lqueue_t *lockqp, bool (*searchfn)(void* elementp,const void* keyp), const void* skeyp) {
    lpq_t *lqp = (lpq_t*)lockqp;
    pthread_mutex_lock(&(lqp->lock));

    void* item = qsearch(lqp->queue, searchfn, skeyp);

    pthread_mutex_unlock(&(lqp->lock));
    return item;
}
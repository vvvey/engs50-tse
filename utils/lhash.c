/* 
 * hash.c -- implements a generic hash table as an indexed set of queues.
 *
 */
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <lqueue.h>
#include <hash.h>
#include <lhash.h>
#include <queue.h>

typedef struct {
    uint32_t size;
    queue_t **table;         // array of queues
    pthread_mutex_t lock;    // mutex for thread safety
} lhasht_t;

/* lhopen -- opens a locked hash table with initial size hsize */
lhashtable_t *lhopen(uint32_t hsize) {
    if (hsize >= 2147483648) {
        return NULL;
    }

    hashtable_t *htp = hopen(hsize);
    if (htp == NULL) {
        return NULL;
    }

    lhasht_t *lhtp = (lhasht_t *)malloc(sizeof(lhasht_t));
    if (lhtp == NULL) {
        hclose(htp);
        return NULL;
    }

    lhtp->table = ((lhasht_t *)htp)->table;
    lhtp->size = ((lhasht_t *)htp)->size;

    pthread_mutex_init(&(lhtp->lock), NULL);

    return (lhashtable_t *)lhtp;
}


/* lhclose -- closes a hash table */
void lhclose(lhashtable_t *lhtp) {
    if (lhtp == NULL) {
        return;
    }

    lhasht_t *lhashtablep = (lhasht_t *)lhtp;

    pthread_mutex_lock(&(lhashtablep->lock));

    hclose((hashtable_t *)lhtp);

    pthread_mutex_unlock(&(lhashtablep->lock));
    pthread_mutex_destroy(&(lhashtablep->lock));
}

/* lhput -- puts an entry into a hash table under designated key 
 * returns 0 for success; non-zero otherwise
 */
int32_t lhput(lhashtable_t *lhtp, void *ep, const char *key, int keylen) {
    if (lhtp == NULL || ep == NULL || key == NULL || keylen < 1) { return -1; }

    lhasht_t *lhashtablep = (lhasht_t *)lhtp;

    pthread_mutex_lock(&(lhashtablep->lock));

    int32_t result = hput((hashtable_t *)lhtp, ep, key, keylen);

    pthread_mutex_unlock(&(lhashtablep->lock));

    return result;
}

/* happly -- applies a function to every entry in hash table */
void lhapply(lhashtable_t *lhtp, void (*fn)(void *ep)) {
    if (lhtp == NULL || fn == NULL) {
        return;
    }

    lhasht_t *lhashtablep = (lhasht_t *)lhtp;

    pthread_mutex_lock(&(lhashtablep->lock));

    happly((hashtable_t *)lhtp, fn);

    pthread_mutex_unlock(&(lhashtablep->lock));
}

/* hsearch -- searchs for an entry under a designated key using a
 * designated search fn -- returns a pointer to the entry or NULL if
 * not found
 */
void *lhsearch(lhashtable_t *lhtp, bool (*searchfn)(void *elementp, const void *searchkeyp), const char *key, int32_t keylen) {
    if (lhtp == NULL || searchfn == NULL || key == NULL || keylen < 1) {
        return NULL;
    }

    lhasht_t *lhashtablep = (lhasht_t *)lhtp;

    pthread_mutex_lock(&(lhashtablep->lock));

    void *result = hsearch((hashtable_t *)lhtp, searchfn, key, keylen);

    pthread_mutex_unlock(&(lhashtablep->lock));

    return result;
}

/* hremove -- removes and returns an entry under a designated key
 * using a designated search fn -- returns a pointer to the entry or
 * NULL if not found
 */
void *lhremove(lhashtable_t *lhtp, bool (*searchfn)(void *elementp, const void *searchkeyp), const char *key, int32_t keylen) {
    if (lhtp == NULL || searchfn == NULL || key == NULL || keylen < 1) {
        return NULL;
    }

    lhasht_t *lhashtablep = (lhasht_t *)lhtp;

    pthread_mutex_lock(&(lhashtablep->lock));

    void *result = hremove((hashtable_t *)lhtp, searchfn, key, keylen);

    pthread_mutex_unlock(&(lhashtablep->lock));

    return result;
}



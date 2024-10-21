/* 
 * hash.c -- implements a generic hash table as an indexed set of queues.
 *
 */
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <hash.h>
#include <queue.h>
/* 
 * SuperFastHash() -- produces a number between 0 and the tablesize-1.
 * 
 * The following (rather complicated) code, has been taken from Paul
 * Hsieh's website under the terms of the BSD license. It's a hash
 * function used all over the place nowadays, including Google Sparse
 * Hash.
 */
#define get16bits(d) (*((const uint16_t *) (d)))

static uint32_t SuperFastHash (const char *data,int len,uint32_t tablesize) {
  uint32_t hash = len, tmp;
  int rem;
  
  if (len <= 0 || data == NULL)
		return 0;
  rem = len & 3;
  len >>= 2;
  /* Main loop */
  for (;len > 0; len--) {
    hash  += get16bits (data);
    tmp    = (get16bits (data+2) << 11) ^ hash;
    hash   = (hash << 16) ^ tmp;
    data  += 2*sizeof (uint16_t);
    hash  += hash >> 11;
  }
  /* Handle end cases */
  switch (rem) {
  case 3: hash += get16bits (data);
    hash ^= hash << 16;
    hash ^= data[sizeof (uint16_t)] << 18;
    hash += hash >> 11;
    break;
  case 2: hash += get16bits (data);
    hash ^= hash << 11;
    hash += hash >> 17;
    break;
  case 1: hash += *data;
    hash ^= hash << 10;
    hash += hash >> 1;
  }
  /* Force "avalanching" of final 127 bits */
  hash ^= hash << 3;
  hash += hash >> 5;
  hash ^= hash << 4;
  hash += hash >> 17;
  hash ^= hash << 25;
  hash += hash >> 6;
  return hash % tablesize;
}

typedef struct {
    uint32_t size;
    queue_t **table; // array of queue
} hasht_t;

/* hopen -- opens a hash table with initial size hsize */
hashtable_t *hopen(uint32_t hsize) {
  if (hsize >= 2147483648) {return NULL;} // if it is negative
  
  hasht_t *htp = (hasht_t*)malloc(sizeof(hasht_t));

  htp->table = (queue_t**)malloc(hsize * sizeof(queue_t*));
  htp->size = (uint32_t)hsize;

  for (uint32_t i = 0; i < hsize; i++) {
    htp->table[i] = (queue_t*)qopen(); // initiate queue
  }

  return (hashtable_t*)htp;
}

/* hclose -- closes a hash table */
void hclose(hashtable_t *htp) {
	if (htp == NULL) {
		return;
	}

	hasht_t *hashtablep = (hasht_t*)htp;
	
	for (uint32_t i = 0; i < hashtablep->size; i++) {
		qclose(hashtablep->table[i]);
	}

  free(hashtablep->table);
	free(hashtablep);
}

/* hput -- puts an entry into a hash table under designated key 
 * returns 0 for success; non-zero otherwise
 */
int32_t hput(hashtable_t *htp, void *ep, const char *key, int keylen) {
  if (htp == NULL || ep == NULL || key == NULL || keylen < 1) { return -1;} // Error
  
  hasht_t *hashtablep = (hasht_t*)htp;

  int loc = (int)SuperFastHash(key, keylen, hashtablep->size);

  queue_t *qp = hashtablep->table[loc]; 

  return qput(qp, ep); // return 0 if qput success
}

/* happly -- applies a function to every entry in hash table */
void happly(hashtable_t *htp, void (*fn)(void* ep)) {
	if (htp == NULL || fn == NULL) {
		return;
	}

	hasht_t *hashtablep = (hasht_t*)htp;

	for (uint32_t i = 0; i < hashtablep->size; i++) {
		qapply(hashtablep->table[i], fn);
	}
}

/* hsearch -- searchs for an entry under a designated key using a
 * designated search fn -- returns a pointer to the entry or NULL if
 * not found
 */
void *hsearch(hashtable_t *htp, bool (*searchfn)(void* elementp, const void* searchkeyp), const char *key, int32_t keylen) {
    if (htp == NULL || searchfn == NULL || key == NULL || keylen < 1) {
        return NULL;
    }

    hasht_t *hashtablep = (hasht_t*)htp;
    int hashloc = (int)SuperFastHash(key, keylen, hashtablep->size);

    queue_t *qp = hashtablep->table[hashloc];

		if (qp == NULL) {
			return NULL;
		}

    return qsearch(qp, searchfn, key);
}


/* hremove -- removes and returns an entry under a designated key
 * using a designated search fn -- returns a pointer to the entry or
 * NULL if not found
 */
void *hremove(hashtable_t *htp, bool (*searchfn)(void* elementp, const void* searchkeyp), const char *key, int32_t keylen) {
    if (htp == NULL || searchfn == NULL || key == NULL || keylen < 1) {
        return NULL;
    }

    hasht_t *hashtablep = (hasht_t*)htp;
    int hashloc = (int)SuperFastHash(key, keylen, hashtablep->size);

    queue_t *qp = hashtablep->table[hashloc];

		if (qp == NULL) {
			return NULL;
		}

    return qremove(qp, searchfn, key);
}


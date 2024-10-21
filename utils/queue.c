/* 
 * queue.c --- 
 * 
 * Author: Samuel R. Hirsh and Vuthy Vey
 * Created: 10-10-2024
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <queue.h>

/* queue node data type */
typedef struct node {
	struct node *next;
	void *data;
} node_t;

/* queue holding two pointers */
typedef struct queue {
	node_t *front;
	node_t *back;
} pq_t;

/* create an empty queue */
queue_t* qopen(void) {
	pq_t *qp = (pq_t*)malloc(sizeof(pq_t));

	if (qp == NULL) {
		return NULL;
	}

	qp->front = NULL;
	qp->back = NULL;
	return (queue_t*)qp;
}


/* deallocate a queue, frees everything in it */
void qclose(queue_t *qp) {
	pq_t *q = (pq_t*)qp;

	node_t *current = q->front;
	node_t *next;

	while (current != NULL) {
		next = current->next;
		free(current);
		current = next;
	}

	free(qp);
}

/* put element at the end of the queue
 * returns 0 is successful; nonzero otherwise 
 */
int32_t qput(queue_t *qp, void *elementp) {
	
	if (qp == NULL || elementp == NULL) {
		return -1;
	}
	//  Cast queue type
	pq_t *the_q = (pq_t *)qp;
	
	// Create new node
	node_t *nodep = (node_t *)malloc(sizeof(node_t));
	if (nodep == NULL) {
		return -1; // Failed to allocate memory
	}

	nodep->data = elementp; // Assign the data to the new node
	nodep->next = NULL; 

	if (the_q->front == NULL && the_q->back == NULL) {
		// Queue is empty
		the_q->front = nodep; 
		the_q->back = nodep;
	} else {
		// Queue not empty
		the_q->back->next = nodep;
		the_q->back = nodep;
	}

	return 0; // Success
}

/* get the first first element from queue, removing it from the queue */
void* qget(queue_t *qp) {
	if (qp == NULL) {
		return NULL;
	}
	
	pq_t *q = (pq_t*)qp;

	if (q->front == NULL) {
		return NULL;
	}
	
	node_t *first = q->front;
	void* data = first->data;
	
	q->front = first->next;

	if (q->front == NULL) {
		q->back = NULL;
	}
	
	free(first);
	
	return data;
}

/* apply a function to every element of the queue */
void qapply(queue_t *qp, void (*fn)(void* elementp)) {
	if (qp == NULL || fn == NULL) {
		return;
  }
  
  pq_t *q = (pq_t*)qp;

	node_t *curr_p = q->front;

  while (curr_p != NULL) {
		fn(curr_p->data);
    curr_p = curr_p->next;
  }
}

/* search a queue using a supplied boolean function
 * skeyp -- a key to search for
 * searchfn -- a function applied to every element of the queue
 *          -- elementp - a pointer to an element
 *          -- keyp - the key being searched for (i.e. will be 
 *             set to skey at each step of the search
 *          -- returns TRUE or FALSE as defined in bool.h
 * returns a pointer to an element, or NULL if not found
 */
void* qsearch(queue_t *qp, bool (*searchfn)(void* elementp,const void* keyp), const void* skeyp) {
	if (qp == NULL) { return NULL;}

	// Cast to queue data type
	pq_t *queue_p = (pq_t*)qp;

	if (queue_p->front == NULL) {return NULL;} 

	// Search for element
	node_t *curr_p = queue_p->front;
	while (curr_p != NULL) {
		void* elem_p = curr_p->data;

		if (searchfn(elem_p, skeyp)) { // return a bool
			return elem_p; // Found
		}

		curr_p = curr_p->next;
	}

	// Not found
	return NULL;
}

/* search a queue using a supplied boolean function (as in qsearch),
 * removes the element from the queue and returns a pointer to it or
 * NULL if not found
 */
void* qremove(queue_t *qp, bool (*searchfn)(void* elementp,const void* keyp), const void* skeyp) {
	if (qp == NULL || searchfn == NULL || skeyp == NULL) {
      return NULL;
  }

  pq_t *q = (pq_t*)qp;

  if (q->front == NULL) {
      return NULL;
  }

	node_t *curr_p = q->front;
  node_t *prev_p = NULL;

  while (curr_p != NULL) {
      void* elem_p = curr_p->data;

      if (searchfn(elem_p, skeyp)) {
          if (prev_p == NULL) {
              q->front = curr_p->next;

							if (q->front == NULL) {
                  q->back = NULL;
              }
          } else {
              prev_p->next = curr_p->next;

              if (curr_p->next == NULL) {
                  q->back = prev_p;
              }
          }

					free(curr_p);
          return elem_p;
      }

      prev_p = curr_p;
      curr_p = curr_p->next;
  }

  return NULL;
}


/* concatenatenates elements of q2 into q1
 * q2 is dealocated, closed, and unusable upon completion 
 */
void qconcat(queue_t *q1p, queue_t *q2p) {


	// Check if q1p and q2p are NULL
    if (q1p == NULL) {
        return; // There's nothing to concatenate to
    }

    if (q2p == NULL) {
        return; // Nothing to concatenate from q2
    }

	pq_t *q1_p = (pq_t*)q1p;
	pq_t *q2_p = (pq_t*)q2p;  
    
    if (q1_p->front == NULL) {
        q1_p->front = q2_p->front;  // set q1's front and back to q2's front and back
        q1_p->back = q2_p->back;
    } else if (q2_p->front != NULL) {
        // Link the end of q1 to the beginning of q2
        q1_p->back->next = q2_p->front; // Connect q1's back to q2's front
        q1_p->back = q2_p->back;        // Update q1's back to q2's back
    }

    // Deallocate q2
    free(q2p);
}


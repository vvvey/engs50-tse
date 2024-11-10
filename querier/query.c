
/* 
 * query.c --- 
 * 
 * Author: Samuel R. Hirsh & Vuthy Vey
 * Created: 11-07-2024
 * Version: 1.0
 * 
 * Description: 
 * 
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <indexio.h>
#include <hash.h>
#include <queue.h>
#include <pageio.h>
#include <webpage.h>


typedef struct {
  queue_t *doc_queue; // queue of documents containing the word
  char *word; // normalized word
} index_t;

typedef struct {
  int doc_id; // document ID
  int count; // word count in the document
} doc_t;

typedef struct {
  int doc_id; // document ID
  int rank; // query rank
  char* url; 
} rank_t;

// Global Variables
queue_t* rank_queue;

/*
* @params sp, a string
* convert string to lower case, only if string contains only alphabet
*/
int NormalizeWord(char *sp) {
  for (int i = 0; sp[i] != '\0'; i++) {
    if (!isalpha(sp[i])) {
      return -1;
    }
    sp[i] = tolower(sp[i]);
  }
  return 0;
}

bool compareWord(void *ip, char *word) {
  index_t *ip1 = (index_t*)(ip);
  return strcmp(ip1->word, word) == 0;
}

bool compareDocID(doc_t *dp, int *doc_id) {
  return dp->doc_id == *doc_id;
}

bool compareDocIDRankQueue(rank_t *rp, int *doc_id) {
  return rp->doc_id == *doc_id;
}

void freeIndex(void *ip) {
	index_t* index = (index_t*)ip;
	
	qapply(index->doc_queue, free);
	qclose(index->doc_queue);
	free(index->word);
	free(index);
}

void freeRank(void *rp) {
	rank_t* rank_p = (rank_t*)rp;
	// free(rank_p->url);
	free(rank_p);
}

void updateRank(void *docp) {
	doc_t *dp = (doc_t*)docp;
	
	int count = dp->count;
	int doc_id = dp->doc_id;

	// rank_queue is a global variable
	
	rank_t *rp = (rank_t*)qsearch(rank_queue, (bool (*)(void *, const void *))compareDocIDRankQueue, &doc_id);
	if (rp == NULL) {
		rp = (rank_t*)malloc(sizeof(rank_t));
		rp->doc_id = doc_id;
		rp->rank = count;
		

		webpage_t *wp = pageload(doc_id, "../crawler/pages");
		if (wp == NULL) {
			printf("yeah it can' be fetch");
		}
		char* url = webpage_getURL(wp);
		
		char* copyURL = malloc(strlen(url)*sizeof(char*));
		strcpy(copyURL, url);
		rp->url = copyURL;
		
		webpage_delete(wp);

		qput(rank_queue, rp);
	} else {
		if (rp->rank > count) {
			rp->rank = count;
		}
	}

	return;
}

void printRank(rank_t *rp) {
	printf("rank: %d doc: %d url: %s\n", rp->rank, rp->doc_id, rp->url);
}

int main() {
	
	hashtable_t *indexer_p = indexload("../indexer/ind");
	if (indexer_p == NULL) {
		printf("Can't load indexer \n");
		return -1;
	}
	
	while(1) {
		rank_queue = qopen();
		char query[1024];

		printf("> ");
		if (scanf(" %[^\n]", query) == EOF) { break; }
		
		char *delimiter = " \t";
		char *word = strtok(query, delimiter); // delimiter space or tabs
	
		int valid = 1;
		int count = 0;
		int capacity = 2;
		char **word_arr = malloc(capacity * sizeof(char*)); // storing each query word
		
		while (word != NULL) {
			
			if (NormalizeWord(word) == -1) {
				valid = 0;
				break;
			}

			if (strlen(word) < 3 || strcmp(word, "and") == 0) {
				word = strtok(NULL, delimiter); // next token
				continue;
			}

			if (count == capacity) { // when count is at capacity --> increase capacity by multiple of 2
            	capacity *= 2;
            	char **temp = realloc(word_arr, capacity * sizeof(char*));
            	
				if (temp == NULL) {
					fprintf(stderr, "Memory reallocation failed\n");
					break;
            	}
            	word_arr = temp;
        	}

			word_arr[count] = malloc((strlen(word) + 1) * sizeof(char));
			strcpy(word_arr[count], word); // Copy the word to array
			count++;

			word = strtok(NULL, delimiter); // next token in scanner
		}
	
		if (valid == 0) {
			printf("Invalid query!\n");
			free(word_arr);
			qclose(rank_queue);
			continue;
		}

		/*
		* Ranking documents based on query
		*/
		for (int i = 0; i < count; i++) {
			char* w = word_arr[i];

			if (strlen(w) < 3) { // Ignore the word
				continue;
			}

			index_t *ip = hsearch(indexer_p,  (bool (*)(void*, const void*))compareWord, w, strlen(w));
			if (ip == NULL) { // word in the query couldn't be found in indexer
				printf("Not found: %s \n", w);
			} else {
				// for each doc_id in doc_queue, update its rank in rank_queue
				queue_t *qp = ip->doc_queue;
				qapply(qp, updateRank); 
			}

			free(w);  // Free each word
    	}
		
		// Print result
		qapply(rank_queue, (void (*)(void *))printRank);

		qapply(rank_queue, freeRank);
		qclose(rank_queue);
		free(word_arr);
	}

	happly(indexer_p, freeIndex);
	hclose(indexer_p);
}


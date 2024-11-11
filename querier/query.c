
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
	
	hashtable_t *indexer_p = indexload("../indexer/test");
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
		char last_word[1024];
	
		int valid = 1;
		
		queue_t *total_parsed = qopen();
		queue_t *connected_words = qopen();

		qput(total_parsed, connected_words);
	
		while (word != NULL) {
			if (NormalizeWord(word) == -1) {
				valid = 0;
				break;
			}

			if ((strcmp(last_word, "and") == 0 || strcmp(last_word, "or") == 0) && (strcmp(word, "and") == 0 || strcmp(word, "or") == 0)) {
				valid = 0;
				break;
			}

			if (strcmp(word, "or") == 0) {
				qput(total_parsed, connected_words);
				strcpy(last_word, word);
				word = strtok(NULL, delimiter);
				connected_words = qopen();
				continue;
			}

			if (strcmp(word, "and") == 0) {
				strcpy(last_word, word);
				word = strtok(NULL, delimiter);				
				continue;
			}
			
			qput(connected_words, word);
			
			strcpy(last_word, word);
			word = strtok(NULL, delimiter);
		}

		if ((strcmp(last_word, "and") == 0 || strcmp(last_word, "or") == 0)) {
			valid = 0;
		}
	
		if (valid == 0) {
			printf("Invalid query!\n");
			qclose(rank_queue);
			continue;
		}
	}

	happly(indexer_p, freeIndex);
	hclose(indexer_p);
}


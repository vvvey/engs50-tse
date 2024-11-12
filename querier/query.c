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
queue_t* rank_queue = NULL;
char *pageDirectory = NULL;

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
	free(rank_p->url);
	free(rank_p);
}

void calcAndRank(void *docp) {
	doc_t *dp = (doc_t*)docp;
	
	int count = dp->count;
	int doc_id = dp->doc_id;

	rank_t *rp = (rank_t*)qsearch(rank_queue, (bool (*)(void *, const void *))compareDocIDRankQueue, &doc_id);
	if (rp == NULL) {
		rp = (rank_t*)malloc(sizeof(rank_t));
		if (rp == NULL) {
			fprintf(stderr, "Memory allocation failed for rank_t\n");
			return;
		}
		rp->doc_id = doc_id;
		rp->rank = count;

		webpage_t *wp = pageload(doc_id, pageDirectory);
		if (wp == NULL) {
			fprintf(stderr, "Error: Could not load page with doc_id %d\n", doc_id);
			free(rp); 
			return;
		}
		char* url = webpage_getURL(wp);
		rp->url = strdup(url);
		webpage_delete(wp); 

		if (rp->url == NULL) {
			fprintf(stderr, "Memory allocation failed for URL\n");
			free(rp); 
			return;
		}

		qput(rank_queue, rp);
	} else {
		if (rp->rank > count) {
			rp->rank = count;
		}
	}
}

void printRank(rank_t *rp) {
	printf("rank: %d doc: %d url: %s\n", rp->rank, rp->doc_id, rp->url);
}

void usage(char *prog_name) {
	printf("Usage: %s <pageDirectory> <indexFile> [-q]\n", prog_name);
}

int main(int argc, char *argv[]) {
 	if (argc < 3 || argc > 4) {
  		usage(argv[0]);
  		return -1;
 	}

 	int quiet_mode = 0;
 	if (argc == 4 && strcmp(argv[3], "-q") == 0) {
 		quiet_mode = 1;
 	}

  	pageDirectory = argv[1];
  	char* indexFile = argv[2];

  	if (access(pageDirectory, F_OK) != 0) {
    	fprintf(stderr, "Error: %s does not exist.\n", pageDirectory);
    	return -1;
  	}
	
	hashtable_t *indexer_p = indexload(indexFile);
	if (indexer_p == NULL) {
		fprintf(stderr, "Can't load indexer\n");
		return -1;
	}
	
	while(1) {
		rank_queue = qopen();
		char query[1024];

		printf("> ");
		if (scanf(" %[^\n]", query) == EOF) { break; }
		
		char *delimiter = " \t";
		char *word = strtok(query, delimiter);
		char last_word[1024] = "";
	
		int valid = 1;
		queue_t *total_parsed = qopen();
		queue_t *connected_words = qopen();
	
		while (word != NULL) {
			if (NormalizeWord(word) == -1) {
				valid = 0;
				break;
			}

			if ((strcmp(last_word, "and") == 0 || strcmp(last_word, "or") == 0) && 
			    (strcmp(word, "and") == 0 || strcmp(word, "or") == 0)) {
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
		
			qput(connected_words, strdup(word));
			
			strcpy(last_word, word);
			word = strtok(NULL, delimiter);
		}

		if ((strcmp(last_word, "and") == 0 || strcmp(last_word, "or") == 0)) {
			valid = 0;
		}

		qput(total_parsed, connected_words);
		
		if (!valid) {
			printf("Invalid query!\n");
			qclose(rank_queue);
			qapply(total_parsed, qclose);
			qclose(total_parsed);
			continue;
		}

		queue_t *andwords = qget(total_parsed);
		queue_t *or_queue = qopen();

		rank_queue = qopen();

		while(andwords != NULL) {
			char *word = qget(andwords);
			while(word != NULL) {
				printf("%s ", word);
				index_t *ip = hsearch(indexer_p,  (bool (*)(void*, const void*))compareWord, word, strlen(word));
				if (ip == NULL) {
					printf("Not found: %s \n", word);
				} else {
					queue_t *qp = ip->doc_queue;
					qapply(qp, calcAndRank);
				}
				free(word);
				word = qget(andwords);
			}
			qclose(andwords);
			qput(or_queue, rank_queue);
			rank_queue = qopen();
			andwords = qget(total_parsed);
		}

		while(qget(or_queue) != NULL) {
			printf("a");
		}

		qapply(rank_queue, freeRank);
		qclose(rank_queue);

		qapply(total_parsed, qclose);
		qclose(total_parsed);
		qclose(or_queue);
	}

	happly(indexer_p, freeIndex);
	hclose(indexer_p);
	return 0;
}

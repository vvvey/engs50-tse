/* 
 * indexer.c --- 
 * 
 * Author: Vuthy Vey
 * Created: 10-25-2024
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pageio.h>
#include <webpage.h>
#include <hash.h>
#include <queue.h>
#include <indexio.h>

int total_count = 0;

typedef struct {
  queue_t *doc_queue; // queue of documents containing the word
  char *word; // normalized word
} index_t;

typedef struct {
  int doc_id; // document ID
  int count; // word count in the document
} doc_t;

bool compareWord(index_t *ip, char *word) {
  return strcmp(ip->word, word) == 0;
}

bool compareDocID(doc_t *dp, int *doc_id) {
  return dp->doc_id == *doc_id;
}

void countQueueSum(doc_t *dp){
  total_count += dp->count;
}

void countTotal(index_t *ip) {
  queue_t *doc_queue = ip->doc_queue;
  qapply(doc_queue, (void (*)(void *))countQueueSum);
}

int NormalizeWord(char *sp) {
  if (strlen(sp) < 3) {
    return -1;
  }

  for (int i = 0; sp[i] != '\0'; i++) {
    if (sp[i] < 65 || sp[i] > 122 || (sp[i] > 90 && sp[i] < 97)) {
      return -1;
    }
    sp[i] = tolower(sp[i]);
  }
  return 0;
}

hashtable_t* index_documents(int end_id, char *page_dir) {
  hashtable_t *index_p = hopen(100);

  for (int doc_id = 1; doc_id <= end_id; doc_id++) {
    webpage_t *wp = pageload(doc_id, page_dir);

		if (!wp) {
			printf("Page could not load");
			continue;
		}

    char *word;
    int pos = 0;

    while ((pos = webpage_getNextWord(wp, pos, &word)) > 0) {
      if (NormalizeWord(word) == 0) {
        index_t *elementp = hsearch(index_p, (bool (*)(void*, const void*))compareWord, word, strlen(word));

        if (elementp == NULL) {
          index_t *ip = (index_t*)malloc(sizeof(index_t));
          ip->doc_queue = qopen();
          ip->word = word;

          doc_t *docp = (doc_t*)malloc(sizeof(doc_t));
          docp->doc_id = doc_id;
          docp->count = 1;
          qput(ip->doc_queue, docp);

          hput(index_p, ip, word, strlen(word));
        } else {
          doc_t *docp = qsearch(elementp->doc_queue, (bool (*)(void*, const void*))compareDocID, &doc_id);

          if (docp == NULL) {
            doc_t *new_docp = (doc_t*)malloc(sizeof(doc_t));
            new_docp->doc_id = doc_id;
            new_docp->count = 1;
            qput(elementp->doc_queue, new_docp);
          } else {
            docp->count++;
          }

					free(word);
        }
      }
    }

		webpage_delete(wp);
  }

  happly(index_p, (void (*)(void *))countTotal);

  printf("Total %d \n", total_count);
  
  return index_p;
}

int main(int argc, char *argv[]) {
	char *pgdir = argv[1];
	char *idfl = argv[2];

	int endid = 7; // ask josh--am i supposed to get the number of files for this?
	hashtable_t *index_p = index_documents(endid, pgdir);

	if (indexsave(index_p, idfl) != 0) {
		exit(EXIT_FAILURE);
	}

	hclose(index_p);
	exit(EXIT_SUCCESS);
}

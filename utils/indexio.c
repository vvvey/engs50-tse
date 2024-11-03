/* 
 * indexio.c --- 
 * 
 * Author: Vuthy Vey
 * Created: 11-03-2024
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <unistd.h>
#include <hash.h>
#include <queue.h>


FILE *fl;

typedef struct {
  queue_t *doc_queue; // queue of documents containing the word
  char *word; // normalized word
} index_t;

typedef struct {
  int doc_id; // document ID
  int count; // word count in the document
} doc_t;

static void formatIndexer(index_t *ip) {
  char *word = ip->word;
  
  fprintf(fl,"%s\n", word);
}

int32_t indexsave(hashtable_t *index_p, char *indexnm) {

    char fp[300];
	sprintf(fp, "%s", indexnm);
	fl = fopen(fp, "w");

	if (fl == NULL) {
		printf("Error opening file");
	}

    happly(index_p, (void (*)(void *))formatIndexer);

    fclose(fl);
    return 0;
}

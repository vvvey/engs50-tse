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


static void fprintDoc(doc_t *dp ) {
  fprintf(fl,"%d %d ", dp->doc_id, dp->count);
}

static void fPrintIndex(index_t *ip) {
  char *word = ip->word;
  queue_t *qp = ip->doc_queue;
  
  fprintf(fl,"%s ", word); // fprint the normalized word
  qapply(qp, (void (*)(void *))fprintDoc); // fpirnt <docID1> <count1> <docID2> <count2> ….<docIDN> <countN>
  fprintf(fl,"\n");
}

/*
 * indexio -- save the indexer 
 *
 * returns: 0 for success; nonzero otherwise
 *
 * The format for the file is:
 *   <word> <docID1> <count1> <docID2> <count2> ….<docIDN> <countN>
 *   <word2> <docID1> <count1> <docID2> <count2> ….<docIDN> <countN>
 *  each entry is seperated by a space
 */
int32_t indexsave(hashtable_t *index_p, char *indexnm) {

  char fp[300];
	sprintf(fp, "%s", indexnm);
	fl = fopen(fp, "w");

	if (fl == NULL) {
		printf("Error opening file");
    return -1;
	}
  
  happly(index_p, (void (*)(void *))fPrintIndex);

  fclose(fl);
  return 0;
}

hashtable_t *indexload(char *indexnm) {
  char fp[300];
	sprintf(fp, "%s", indexnm);
	FILE *fl = fopen(fp, "r");

	if (fl == NULL) {
		printf("Error opening file");
    return NULL;
	}

  hashtable_t *index_p = hopen(100);
  if (index_p == NULL) { return NULL; }

  char *word = malloc(100);
  while (fscanf(fl, "%99s", word) == 1) {
    char *word_copy = malloc(strlen(word) + 1); 
    if (word_copy == NULL) { return NULL; }
    strcpy(word_copy, word); // Copy the content

    index_t *ip = (index_t *)malloc(sizeof(index_t));
    queue_t *qp = qopen();
    ip->word = word_copy;
    ip->doc_queue = qp;

    int id, count;
    while (fscanf(fl, "%d %d", &id, &count) == 2) {
      doc_t *dp = (doc_t*)malloc(sizeof(doc_t));
      dp->doc_id = id;
      dp->count = count;

      qput(qp, dp);
    }

    hput(index_p, ip, word_copy, strlen(word_copy));

  } 
  fclose(fl);
  return index_p;
}
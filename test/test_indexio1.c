/* 
 * test_indexio1.c --- 
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
#include <stdbool.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
// Custom headers
#include <webpage.h>
#include <indexio.h>

typedef struct {
  queue_t *doc_queue; // queue of documents containing the word
  char *word; // normalized word
} index_t;

typedef struct {
  int doc_id; // document ID
  int count; // word count in the document
} doc_t;

bool searchIndex(index_t *ip, char *word) {
    return (strcmp(ip->word, word) == 0);
}

int main(){
  hashtable_t *indexer = hopen(100);

  index_t *i1 = (index_t*)malloc(sizeof(index_t));
  i1->word = "hello";
  i1->doc_queue = qopen();

  doc_t *d1 = (doc_t*)malloc(sizeof(doc_t));
  d1->doc_id = 1;
  d1->count = 10;

  doc_t *d2 = (doc_t*)malloc(sizeof(doc_t));
  d2->doc_id = 2;
  d2->count = 3;

  qput(i1->doc_queue, d1);
  qput(i1->doc_queue, d2);

  index_t *i2 = (index_t*)malloc(sizeof(index_t));
  i2->word = "world";
  i2->doc_queue = qopen();

  doc_t *d3 = (doc_t*)malloc(sizeof(doc_t));
  d3->doc_id = 1;
  d3->count = 2;

  doc_t *d4 = (doc_t*)malloc(sizeof(doc_t));
  d4->doc_id = 2;
  d4->count = 1;

  qput(i2->doc_queue, d3);
  qput(i2->doc_queue, d4);

  hput(indexer, i1, "jellys", 6);
  hput(indexer, i2, "worlds", 6);

  indexsave(indexer, "ind");

  hashtable_t *new_in = indexload("ind");
  index_t *new_i = (index_t*)hsearch(new_in, (bool (*)(void *, const void *))searchIndex, "hello", 5);
  index_t *new_i1 = (index_t*)hsearch(new_in, (bool (*)(void *, const void *))searchIndex, "world", 5);
  

  printf("%s \n", new_i->word);

  // Test for word
  if (new_in == NULL || new_i == NULL || new_i1 == NULL || strcmp(new_i->word, "hello") != 0 || strcmp(new_i1->word, "world") != 0) {
    exit(EXIT_FAILURE);
  } 

  queue_t *q1 = new_i->doc_queue;
  doc_t *nd1 = qget(q1);
  doc_t *nd2 = qget(q1);

  if (q1 == NULL || nd1 == NULL || nd2 == NULL || nd1->doc_id != 1 || nd1->count != 10 || nd2->doc_id != 2 || nd2->count != 3) {
    exit(EXIT_FAILURE);
  } 

  queue_t *q2 = new_i1->doc_queue;
  doc_t *nd3 = qget(q2);
  doc_t *nd4 = qget(q2);

  if (q2 == NULL || nd3 == NULL || nd4 == NULL || nd3->doc_id != 1 || nd3->count != 2 || nd4->doc_id != 2 || nd4->count != 1) {
    exit(EXIT_FAILURE);
  } 

  exit(EXIT_SUCCESS);

} 
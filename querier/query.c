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
char *pageDirectory;
int quiet_mode = 0;
FILE *outFile = NULL; // Global file pointer for output

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
    free(rank_p);
}

void calcAndRank(void *docp) {
    doc_t *dp = (doc_t*)docp;
    int count = dp->count;
    int doc_id = dp->doc_id;

    rank_t *rp = (rank_t*)qsearch(rank_queue, (bool (*)(void *, const void *))compareDocIDRankQueue, &doc_id);
    if (rp == NULL) {
        rp = (rank_t*)malloc(sizeof(rank_t));
        rp->doc_id = doc_id;
        rp->rank = count;

        webpage_t *wp = pageload(doc_id, pageDirectory);
        if (wp == NULL) {
            printf("Page could not be fetched\n");
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
}

void printRank(rank_t *rp) {
    printf("rank: %d doc: %d url: %s\n", rp->rank, rp->doc_id, rp->url);
}

void saveRank(rank_t *rp) {
    if (outFile != NULL) {
        fprintf(outFile, "rank: %d doc: %d url: %s\n", rp->rank, rp->doc_id, rp->url);
    }
}

bool compareRankID(rank_t *rp, char *strid) {
    char doc_idstr[100];
    sprintf(doc_idstr, "%d", rp->doc_id);
    return strcmp(doc_idstr, strid) == 0;
}

void usage() {
    printf("usage: query pageDirectory indexFile [-q file]\n");
}

void processQuery(char *query, hashtable_t *indexer_p) {
    rank_queue = qopen();
    char *delimiter = " \t";
    char *word = strtok(query, delimiter);
    char last_word[1024];
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
        qput(connected_words, word);
        strcpy(last_word, word);
        word = strtok(NULL, delimiter);
    }
    if ((strcmp(last_word, "and") == 0 || strcmp(last_word, "or") == 0)) {
        valid = 0;
    }
    qput(total_parsed, connected_words);

    if (valid == 0) {
        printf("Invalid query!\n");
        qclose(rank_queue);
        return;
    }

    queue_t *andwords = qget(total_parsed);
    queue_t *or_queue = qopen();

    rank_queue = qopen();

    while (andwords != NULL) {
        char *word = qget(andwords);
        int allWordsPass = 1;
        while (word != NULL) {
            index_t *ip = hsearch(indexer_p, (bool (*)(void*, const void*))compareWord, word, strlen(word));
            if (ip == NULL) {
                allWordsPass = 0;
                break;
            } else {
                queue_t *qp = ip->doc_queue;
                qapply(qp, calcAndRank); 
            }
            word = qget(andwords);
        }
        if (allWordsPass == 1) {
            qput(or_queue, rank_queue);
            rank_queue = qopen();
        }
        andwords = qget(total_parsed);
    }

    hashtable_t *rank_result = hopen(100);

    queue_t *tmp = qget(or_queue);
    while (tmp != NULL) {
        rank_t *rp = qget(tmp);
        while (rp != NULL) {
            int doc_id = rp->doc_id;
            int rank = rp->rank;

            char doc_idstr[100];
            sprintf(doc_idstr, "%d", doc_id);

            rank_t *r_tmp = hsearch(rank_result, (bool (*)(void*, const void*))compareRankID, doc_idstr, strlen(doc_idstr));
            if (r_tmp == NULL) {
                hput(rank_result, rp, doc_idstr, strlen(doc_idstr));
            } else {
                r_tmp->rank = r_tmp->rank + rank;
            }
            rp = qget(tmp);
        }
        tmp = qget(or_queue);
    }

    if (quiet_mode) {
        happly(rank_result, saveRank);
        fprintf(outFile, "\n");
    } else {
        happly(rank_result, printRank);
    }

}

int main(int argc, char *argv[]) {
    if (argc < 3 || argc > 6) {
        usage();
        return -1;
    }

    FILE *query_file = NULL;

    if (argc == 6 && strcmp(argv[3], "-q") == 0) {
        quiet_mode = 1;
        query_file = fopen(argv[4], "r");
        outFile = fopen(argv[5], "w");
        if (query_file == NULL || outFile == NULL) {
            printf("Error: Unable to open query or output file %s.\n", argv[4]);
            return -1;
        }
    }

    pageDirectory = argv[1];
    char *indexFile = argv[2];
    
    if (access(pageDirectory, F_OK) != 0) {
        printf("Error: %s does not exist.\n", pageDirectory);
        return -1;
    }
    
    hashtable_t *indexer_p = indexload(indexFile);

    if (indexer_p == NULL) {
        printf("Can't load indexer \n");
        return -1;
    }

    if (query_file) {
        char query[1024];
        while (fgets(query, sizeof(query), query_file)) {
            query[strcspn(query, "\n")] = '\0';
            processQuery(query, indexer_p);
        }
        fclose(query_file);
        fclose(outFile); // Close outFile after all queries
    } else {
        while (1) {
            printf("> ");
            char query[1024];
            if (scanf(" %[^\n]", query) == EOF) break;
            processQuery(query, indexer_p);
        }
    }

    happly(indexer_p, freeIndex);
    hclose(indexer_p);
    return 0;
}


/* 
 * query.c --- 
 * 
 * Author: Samuel R. Hirsh
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


typedef struct {
  queue_t *doc_queue; // queue of documents containing the word
  char *word; // normalized word
} index_t;

typedef struct {
  int doc_id; // document ID
  int count; // word count in the document
} doc_t;

int NormalizeWord(char *sp) {
//   if (strlen(sp) < 3) {
//     return -1;
//   }

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


int main() {
	hashtable_t *indexer_p = indexload("../indexer/ind");
	if (indexer_p == NULL) {
		printf("Can't load indexer \n");
		return -1;
	}
	
	while(1) {

		char query[1024];

		printf("> ");
		if (scanf(" %[^\n]", query) == EOF) {
			break;
        }
		
		char *delimiter = " \t";
		char *word = strtok(query, delimiter); // delimiter space or tabs
	
		int valid = 1;
		int count = 0;
		int capacity = 2;
		char **word_arr = malloc(capacity * sizeof(char*));
		
		while (word != NULL) {
			
			if (NormalizeWord(word) == -1) {
				valid = 0;
				break;
			}

			if (strlen(word) < 3 || strcmp(word, "and") == 0) {
				word = strtok(NULL, delimiter); // next token
				continue;
			}

			if (count == capacity) {
            	capacity *= 2;
            	char **temp = realloc(word_arr, capacity * sizeof(char*));
            	
				if (temp == NULL) {
					fprintf(stderr, "Memory reallocation failed\n");
					break;
            	}
            	word_arr = temp;
        	}

			word_arr[count] = malloc((strlen(word) + 1) * sizeof(char));
			strcpy(word_arr[count], word); // Copy the content
			count++;

			word = strtok(NULL, delimiter); // next token
		}
	

		if (valid == 0) {
			printf("Invalid query!\n");
			free(word_arr);
			continue;
		}

		int rank = 10000; // maximum possible
		for (int i = 0; i < count; i++) {
			char* w = word_arr[i];

			if (strlen(w) < 3) { // Ignore the word
				continue;
			}

			
			index_t *ip = hsearch(indexer_p,  (bool (*)(void*, const void*))compareWord, w, strlen(w));
			if (ip == NULL) { 
				printf("%s:%d ", w, 0);
				rank = 0;
			} else {
				int doc_id = 1;
				doc_t *dp = qsearch(ip,  (bool (*)(void*, const void*))compareDocID, &doc_id);
				int d = 0;
				if (dp != NULL) {
					d = dp->count;
				}
				
				if (d < rank) { rank = d;}

				printf("%s:%d ", w, d);
			}

			
			free(w);  // Free each word
    	}
		printf("-- %d\n", rank);

		free(word_arr);
	}
}


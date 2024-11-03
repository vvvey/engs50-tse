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

int total_count = 0;

typedef struct {
    int count;
    char *word; // normalized word
} index_t;

bool compareWord(index_t *ip, char* word) {
	return strcmp(ip->word, word) == 0;
}

void countTotal(index_t *ip) {
    total_count += ip->count;
}

int NormalizeWord(char *sp) {
    if (strlen(sp) < 4) { 
        return -1; // return Failure
    }
    for (int i = 0; sp[i] != '\0'; i++) {
        if (sp[i] < 65 || sp[i] > 122 || (sp[i] > 90 && sp[i] < 97)) { // Non alphabet
            return -1;
        }

        sp[i] = tolower(sp[i]);
    }
    return 0;
}

int main() {

    // webpage_t *wp = webpage_new("https://thayer.github.io/engs50/", 0, NULL);
    // webpage_fetch(wp);

    webpage_t *wp = pageload(1, "../crawler/pages");
    hashtable_t *index_p = hopen(100);


    char *word;
    int pos = 0;
    while ((pos = webpage_getNextWord(wp, pos, &word)) > 0) {

        if (NormalizeWord(word) == 0) {
            printf("%s \n", word);

            index_t *elementp = hsearch(index_p, (bool (*)(void*, const void*))compareWord, word, strlen(word));
            if (elementp == NULL) { // Word doesn't exist in hash table
                index_t *ip = (index_t*)malloc(sizeof(index_t));
                ip->count = 1;
                ip->word = word;
                hput(index_p, ip, word, strlen(word)); 
			} else {
                elementp->count = (elementp->count)+1; // Word exist in hash table so increment count by 1
                free(word);
			}
        }   
    }

    happly(index_p, (void (*)(void *))countTotal );
    printf("Total %d \n", total_count);
    
    
    webpage_delete(wp);
    exit(EXIT_SUCCESS);
}

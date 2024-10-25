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

    char *s;
    int pos = 0;
    while ((pos = webpage_getNextWord(wp, pos, &s)) > 0) {

        if (NormalizeWord(s) == 0) {printf("%s \n", s);}
 
        free(s);
    }`
    
    webpage_delete(wp);
    exit(EXIT_SUCCESS);
}
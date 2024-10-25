/* 
 * pageio.c --- 
 * 
 * Author: 
 * Created: 10-24-2024
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
#include <pageio.h>


int main() {
    
    webpage_t *hp = webpage_new("https://thayer.github.io/engs50/", 0, NULL);
    webpage_fetch(hp);

    const char *dir_name = "pages";

    // Check if the directory exists
    struct stat st;
    if (stat(dir_name, &st) == -1) {
        if (mkdir(dir_name, 0755) == -1) {
            exit(EXIT_FAILURE);
        }
    } 

    pagesave(hp, 1, "pages");
    webpage_t *new_hp = pageload(1, "pages");

    if (new_hp == NULL || strcmp(webpage_getHTML(hp), webpage_getHTML(new_hp)) != 0 || strcmp(webpage_getURL(hp), webpage_getURL(new_hp)) != 0 || webpage_getDepth(hp) == webpage_getDepth(new_hp)   ) {
        webpage_delete(hp);
        webpage_delete(new_hp);
        
        exit(EXIT_FAILURE);
    } else {
        webpage_delete(hp);
        webpage_delete(new_hp);

        exit(EXIT_SUCCESS);
    }
    
}
/* 
 * crawler.c --- 
 * 
 * Author: Samuel R. Hirsh & Vuthy Vey
 * Created: 10-21-2024
 * Version: 1.0
 * 
 * Description: 
 * 
 */


#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <string.h>

// Custom headers
#include <webpage.h>
#include <queue.h>
#include <hash.h>
#include <pageio.h>


void printURL(webpage_t *wp) {
	char* urlp = webpage_getURL(wp);
	int depth = webpage_getDepth(wp);

	printf("Depth: %d, url: %s \n", depth, urlp);
	//	free(urlp);
}

bool compareURL(webpage_t *wp, char* url) {
	char* urlp = webpage_getURL(wp);
	return strcmp(urlp, url) == 0;
}

int  main(int argc, char *argv[]) {
	// Check arguments
	if (argc != 4) {
		printf("Usage: crawler <seedurl> <pagedir> <maxdepth> \n ");
		exit(EXIT_FAILURE);
	}

	char *ep; 
	char *seedurl = argv[1];
	char *pagedir = argv[2];
	int maxdepth = strtod(argv[3], &ep);
	
	printf("%s %s %d \n", seedurl, pagedir, maxdepth);

	// Check if page dir exists
	struct stat sb;

    if (stat(pagedir, &sb) != 0) {
		printf("%s directory doesn't exist \n", pagedir);
        printf("Usage: crawler <seedurl> <pagedir> <maxdepth> \n ");
		exit(EXIT_FAILURE);
    } 

	// check if maxdepth is positive
	if (strcmp(ep, "") != 0 || maxdepth < 0) {
		printf("Usage: crawler <seedurl> <pagedir> <maxdepth> \n ");
		exit(EXIT_FAILURE);
	}

	//char url[] = "https://thayer.github.io/engs50/"; 
	int depth = 0;
	webpage_t* homepage_p = webpage_new(seedurl, depth, NULL);

	queue_t *wp_qp = qopen();
	hashtable_t *wp_hp = hopen(100);

	// Add homepage to queue and hash
	qput(wp_qp, homepage_p);
	hput(wp_hp, homepage_p, seedurl, strlen(seedurl));
	
	webpage_t *currp = (webpage_t*)qget(wp_qp);
	int file_id = 1;
	
	while (currp != NULL) {

		bool isFetch = webpage_fetch(currp);
		if (isFetch == false) { // Can't fetch page
			printf("Can't fetch: %s \n", webpage_getURL(currp));
			currp = qget(wp_qp);
			continue;
		}

		// Save page
		printf("Saving: %s \n", webpage_getURL(currp));
		pagesave(currp, file_id, pagedir);
		int currdepth = webpage_getDepth(currp);

		int pos = 0;
		char *result;
		
		while(currdepth < maxdepth && (pos = webpage_getNextURL(currp, pos, &result)) > 0) {
			if (IsInternalURL(result)) {

				webpage_t *wp = webpage_new(result, currdepth + 1, NULL);

				if (hsearch(wp_hp, (bool (*)(void*, const void*))compareURL, result, strlen(result)) == NULL) {
					qput(wp_qp, wp);
					hput(wp_hp, wp, result, strlen(result));
				} else {
					webpage_delete(wp);
				}
			} 

			free(result);
		}	

		file_id = file_id + 1;
		currp = qget(wp_qp);
	}

	//	qapply(wp_qp, webpage_delete);
	happly(wp_hp, webpage_delete);
	webpage_delete(currp);
	
	qclose(wp_qp);
	hclose(wp_hp);
	
	//	webpage_delete(homepage_p);
	
	exit(EXIT_SUCCESS);
}

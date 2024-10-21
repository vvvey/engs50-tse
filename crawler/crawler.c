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
#include <webpage.h>
#include <string.h>
#include <queue.h>

void printURL(webpage_t *wp) {
	char* urlp = webpage_getURL(wp);
	int depth = webpage_getDepth(wp);

	printf("Depth: %d, url: %s \n", depth, urlp);
	free(urlp);
}


int  main() {
	char url[] = "https://thayer.github.io/engs50/"; 
	const int depth = 0;
	
	webpage_t* homepage_p = webpage_new(url, depth, NULL);

	if (homepage_p == NULL) { // Can't open a webpage
		free(homepage_p);
		exit(EXIT_FAILURE);
	}

	bool isFetch = webpage_fetch(homepage_p);
	if (isFetch == false) { // Can't fetch page
		free(homepage_p);
		exit(EXIT_FAILURE);
	}

	char* html = webpage_getHTML(homepage_p);
	//printf("[+] Successfully got webpage\n");
	//printf("%s", html);

	int pos = 0;
	char *result;

	queue_t *wp_qp = qopen();

	while((pos = webpage_getNextURL(homepage_p, pos, &result)) > 0) {
		if (IsInternalURL(result)) {

			webpage_t *wp = webpage_new(result, webpage_getDepth(homepage_p) + 1, NULL);
			qput(wp_qp, wp);
			
			printf("Found internal URL: ");
		} else {
			printf("Found external URL: ");
		}
		
		printf("%s\n", result);
		free(result);
	}
	
	printf("\n");
	printf("URLs in queue: \n");

	// Print all the URLs in the Queue
	qapply(wp_qp, (void (*)(void *))printURL); 
	qclose(wp_qp);

	free(homepage_p);
	free(html);

	exit(EXIT_SUCCESS);
}

/* 
 * crawler.c --- 
 * 
 * Author: Samuel R. Hirsh
 * Created: 10-21-2024
 * Version: 1.0
 * 
 * Description: 
 * 
 */


#include <stdio.h>
#include <webpage.h>
#include <string.h>

int  main() {
	char url[] = "https://thayer.github.io/engs50/"; 
	const int depth = 0;
	
	webpage_t* webpage = webpage_new(url, depth, NULL);
	webpage_fetch(webpage);
	char* html = webpage_getHTML(webpage);
	
	if (webpage != NULL) {
		printf("[+] Successfully got webpage\n");
		printf("%s", html);
	} else {
		exit(EXIT_FAILURE);
	}

	int pos = 0;
	char *result;

	while((pos = webpage_getNextURL(webpage, pos, &result)) > 0) {
		if (IsInternalURL(result)) {
			printf("Found internal URL: ");
		} else {
			printf("Found external URL: ");
		}
		
		printf("%s\n", result);
	}

	free(result);
	free(webpage);
	free(html);
}

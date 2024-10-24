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

#include <pageio.h>
#include <webpage.h>

int32_t pagesave(webpage_t *pagep, int id, char *dirname) {
	char* urlp = webpage_getURL(pagep);
	int depth = webpage_getDepth(pagep);
	char* html = webpage_getHTML(pagep);
	int len = strlen(html);

	char fp[300];
	sprintf(fp, "%s/%d", dirname, id);
	FILE *fl = fopen(fp, "w");

	if (fl == NULL) {
		printf("Error opening file");
	}
	
	fprintf(fl, "%s\n%d\n%d\n%s", urlp, depth, len, html);
	fclose(fl);

	return 0;
}

webpage_t *pageload(int id, char *dirnm) {
	return NULL;
}

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
#include <stdio.h>
#include <stdlib.h>

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
	
	fprintf(fl,"%s\n%d\n%d\n%s", urlp, depth, len, html);
	fclose(fl);

	return 0;
}

webpage_t *pageload(int id, char *dirnm) {
  char fp[256];
  snprintf(fp, sizeof(fp), "%s/%d", dirnm, id);

  FILE *fl = fopen(fp, "r");
  if (!fl) return NULL;

  char url[1024];

	if (fscanf(fl, "%1023s\n", url) != 1) {
    fclose(fl);
    return NULL;
  }

  int depth;
  int html_len;

	if (fscanf(fl, "%d\n%d\n", &depth, &html_len) != 2) {
    fclose(fl);
    return NULL;
  }

	char line[1024];
	char html[html_len * 1024];
	
	while (fscanf(fl,"%1023s\n", line) != 1) {
		strcat(html, line);
	}
	
  webpage_t *page = webpage_new(url, depth, html);

	if (!page) {
     return NULL;
  }

  return page;
}


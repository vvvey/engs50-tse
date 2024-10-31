/* 
 * pageio.c --- 
 * 
 * Author: Sam Hirsh & Vuthy Vey
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
	int len = webpage_getHTMLlen(pagep);

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
  int depth;
  int html_len;

  if (fscanf(fl, "%[^\n]\n%d\n%d\n", url, &depth, &html_len) != 3) {
    fclose(fl);
    return NULL;
  }

  char *html = malloc(html_len + 1); 
  if (!html) {
	fclose(fl);
    return NULL; 
  }

  for (int index = 0; index < html_len; index = index + 1) {
	int ch = fgetc(fl);
	
	html[index] = (char)ch;
  }
	html[html_len] = '\0';
	fclose(fl);
  
  webpage_t *page = webpage_new(url, depth, html);

	if (!page) {
     return NULL;
  }

  return page;
}


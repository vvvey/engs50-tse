
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

int main() {
	while(1) {
		char out[1024];
		out[0] = '\0';

		char query[1024];

		printf("> ");
		scanf(" %[^\n]", query);
	
		char *word = strtok(query, " ");
		int valid = 1;
		
		while (word != NULL) {
			for (int i = 0; word[i] != '\0'; i++) {
				if (isalpha(word[i])) {
					char lower = tolower(word[i]);
					strncat(out, &lower, 1);
				} else {
					valid = 0;
					break;
				}
			}
    
			strncat(out, " ", 1);

			word = strtok(NULL, " ");
		}

		if (valid == 0) {
			printf("Invalid query!\n");
			continue;
		}

		size_t len = strlen(out);
		
		if (len > 0 && out[len - 1] == ' ') {
			out[len - 1] = '\0';
		}

		printf("%s\n", out);
	}
}


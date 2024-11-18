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
#include <pthread.h>
#include <lqueue.h>

pthread_mutex_t fmutex;
int file_id = 1; // global variable

pthread_mutex_t pmutex;
bool page_retrieved = true;

typedef struct {
    lqueue_t *wp_qp;
    hashtable_t *wp_hp;
    int maxdepth;
    char *pagedir;
	int threadid;
} crawl_arg_t;

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

void* crawlWebpage(crawl_arg_t *cr) {
	lqueue_t *wp_qp = cr->wp_qp;
    hashtable_t *wp_hp = cr->wp_hp;
    int maxdepth = cr->maxdepth;
    char *pagedir = cr->pagedir;
	int threadid = cr->threadid;

	webpage_t *currp = (webpage_t*)lqget(wp_qp);

	while (currp != NULL || page_retrieved == false) {
		if (currp == NULL) {
			currp = lqget(wp_qp);
			continue;
		}

		bool isFetch = webpage_fetch(currp);
		if (isFetch == false) { // Can't fetch page
			printf("Can't fetch: %s \n", webpage_getURL(currp));
			currp = lqget(wp_qp);
			continue;
		}

		// Save page
		printf("PID%d: Saving: %s \n", threadid, webpage_getURL(currp));
		pagesave(currp, file_id, pagedir);
		int currdepth = webpage_getDepth(currp);

		int pos = 0;
		char *result;


		pthread_mutex_lock(&pmutex);

        // Critical section
        page_retrieved = false;
 
		while(currdepth < maxdepth && (pos = webpage_getNextURL(currp, pos, &result)) > 0) {
			if (IsInternalURL(result)) {

				webpage_t *wp = webpage_new(result, currdepth + 1, NULL);

				if (hsearch(wp_hp, (bool (*)(void*, const void*))compareURL, result, strlen(result)) == NULL) {
					lqput(wp_qp, wp);
					hput(wp_hp, wp, result, strlen(result));
				} else {
					webpage_delete(wp);
				}
			} 

			free(result);
		}	
		page_retrieved = true; // might need mutex
		// Unlock the mutex after accessing the global variable
        pthread_mutex_unlock(&pmutex);

		pthread_mutex_lock(&fmutex);

        // Critical section
        file_id = file_id + 1;

        // Unlock the mutex after accessing the global variable
        pthread_mutex_unlock(&fmutex);

		
		currp = lqget(wp_qp);
	}
	webpage_delete(currp);

	return NULL;
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

	int num_threads = 4;
	
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
	webpage_t* wiki_p = webpage_new("https://en.wikipedia.org/wiki/Dartmouth_College", 0, NULL);

	lqueue_t *wp_qp = lqopen();
	hashtable_t *wp_hp = hopen(100);
	file_id = 1; // Global variable // with Mutex
	page_retrieved = false; // Global variable

	if (pthread_mutex_init(&fmutex, NULL) != 0) {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }

	if (pthread_mutex_init(&pmutex, NULL) != 0) {
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }

	// Add homepage to queue and hash
	lqput(wp_qp, homepage_p);
	lqput(wp_qp, wiki_p);
	hput(wp_hp, homepage_p, seedurl, strlen(seedurl));
	hput(wp_hp, wiki_p, "https://en.wikipedia.org/wiki/Dartmouth_College", strlen("https://en.wikipedia.org/wiki/Dartmouth_College"));
	
	// Crawl
	crawl_arg_t *cr = (crawl_arg_t*)malloc(sizeof(crawl_arg_t));
	cr->wp_hp = wp_hp;
	cr->wp_qp = wp_qp;
	cr->maxdepth = maxdepth;
	cr->pagedir = pagedir;
	cr->threadid = 1;

	

	pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    if (threads == NULL) {
        printf("Failed to allocate memory for threads");
        exit(EXIT_FAILURE);
    }

	for (int i = 0; i < num_threads; ++i) {
		crawl_arg_t *cr = (crawl_arg_t*)malloc(sizeof(crawl_arg_t));
		cr->wp_hp = wp_hp;
		cr->wp_qp = wp_qp;
		cr->maxdepth = maxdepth;
		cr->pagedir = pagedir;
		cr->threadid = i;

        if (pthread_create(&threads[i], NULL,  (void * (*)(void *))crawlWebpage, (void *)cr) != 0) {
            perror("Failed to create thread");
            free(threads);  // Clean up allocated memory before exiting
            exit(EXIT_FAILURE);
        }
    }


    for (int i = 0; i < num_threads; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join thread");
            free(threads);  // Ensure memory is cleaned on exit
            exit(EXIT_FAILURE);
        }
    }

	lqapply(wp_qp, webpage_delete);

	pthread_mutex_destroy(&fmutex);
	pthread_mutex_destroy(&pmutex);

	happly(wp_hp, webpage_delete);
	
	qclose(wp_qp);
	hclose(wp_hp);
	
	//	webpage_delete(homepage_p);
	
	exit(EXIT_SUCCESS);

}

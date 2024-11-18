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
#include <lhash.h>

pthread_mutex_t fmutex;
int file_id = 1; // global variable

pthread_mutex_t pmutex;
bool page_retrieving = false;
int num_page_retrieved = 0;
int num_page_saved = 0;

typedef struct {
    lqueue_t *wp_qp;
    lhashtable_t *wp_hp;
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
    lhashtable_t *wp_hp = cr->wp_hp;
    int maxdepth = cr->maxdepth;
    char *pagedir = cr->pagedir;
	int threadid = cr->threadid;

	webpage_t *currp = (webpage_t*)lqget(wp_qp);

	while (currp != NULL || page_retrieving == true) {
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
		pthread_mutex_lock(&fmutex);
		int local_file_id = file_id;
		file_id = file_id + 1;
		pthread_mutex_unlock(&fmutex);

		pagesave(currp, local_file_id, pagedir);
		
		
		int currdepth = webpage_getDepth(currp);

		int pos = 0;
		char *result;

        page_retrieving = true;
		while(currdepth < maxdepth && (pos = webpage_getNextURL(currp, pos, &result)) > 0) {
			if (IsInternalURL(result)) {

				webpage_t *wp = webpage_new(result, currdepth + 1, NULL);

				if (lhsearch(wp_hp, (bool (*)(void*, const void*))compareURL, result, strlen(result)) == NULL) {
					lqput(wp_qp, wp);
					lhput(wp_hp, wp, result, strlen(result));				
				} else {
					webpage_delete(wp);
				}
			} 

			free(result);
		}	
		page_retrieving = false; // might need mutex
		currp = lqget(wp_qp);
	}
	webpage_delete(currp);
	free(cr);

	return NULL;
}


int  main(int argc, char *argv[]) {
	// Check arguments
	if (argc != 5) {
		printf("Usage: crawler <seedurl> <pagedir> <maxdepth> <numpthread>\n ");
		exit(EXIT_FAILURE);
	}

	char *ep; 
	char *seedurl = argv[1];
	char *pagedir = argv[2];
	int maxdepth = strtod(argv[3], &ep);
	int num_threads = strtod(argv[4], &ep);
	
	printf("%s %s %d %d\n", seedurl, pagedir, maxdepth, num_threads);

	// Check if page dir exists
	struct stat sb;

    if (stat(pagedir, &sb) != 0) {
		printf("%s directory doesn't exist \n", pagedir);
        printf("Usage: crawler <seedurl> <pagedir> <maxdepth> <numpthread>\n  ");
		exit(EXIT_FAILURE);
    } 

	// check if maxdepth is positive
	if (strcmp(ep, "") != 0 || maxdepth < 0) {
		printf("Usage: crawler <seedurl> <pagedir> <maxdepth> <numpthread>\n ");
		exit(EXIT_FAILURE);
	}

	//char url[] = "https://thayer.github.io/engs50/"; 
	int depth = 0;
	webpage_t* homepage_p = webpage_new(seedurl, depth, NULL);

	lqueue_t *wp_qp = lqopen();
	lhashtable_t *wp_hp = lhopen(100);

	// Add homepage to queue and hash
	lqput(wp_qp, homepage_p);
	lhput(wp_hp, homepage_p, seedurl, strlen(seedurl));


	// Thread Stuffs
	file_id = 1; // Global variable // with Mutex
	page_retrieving = true; // Global variable

	if (pthread_mutex_init(&fmutex, NULL) != 0) { // mutex for file_id (count)
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }

	if (pthread_mutex_init(&pmutex, NULL) != 0) { // mutex for page_retrieved (status to tell if page is being retrieve)
        perror("Failed to initialize mutex");
        exit(EXIT_FAILURE);
    }
	
	pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    if (threads == NULL) {
        printf("Failed to allocate memory for threads");
        exit(EXIT_FAILURE);
    }

	// Create Thread
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

	// Join Threads
    for (int i = 0; i < num_threads; ++i) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("Failed to join thread");
            free(threads);  // Ensure memory is cleaned on exit
            exit(EXIT_FAILURE);
        }
    }

	pthread_mutex_destroy(&fmutex);
	pthread_mutex_destroy(&pmutex);


	// Freeing stuffs
	lqapply(wp_qp, webpage_delete);
	lhapply(wp_hp, webpage_delete);	
	qclose(wp_qp);
	lhclose(wp_hp);
	
	exit(EXIT_SUCCESS);
}

/* 
 * test_lqueue1.c --- 
 * 
 * Author: Vuthy Vey
 * Created: 11-17-2024
 * Version: 1.0
 * 
 * Description: 
 * 
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <lqueue.h>
#include <unistd.h>
#include <pthread.h>

#define MAXREG 10

/* the representation of a car */
typedef struct car {
	char plate[MAXREG];
	double price;
	int year;
} car_t;

car_t *make_car(char *platep, double price, double year) {
    car_t *cp;

    if (!(cp = (car_t *)malloc(sizeof(car_t)))) {
        printf("[Error: malloc failed allocating car]\n");
        return NULL;
    }

    strcpy(cp->plate, platep);
    cp->price = price;
    cp->year = year;
    return cp;
}

void printCar(void *carp) {
    car_t *cp = (car_t*)carp;
    printf("%s %f %d \n", cp->plate, cp->price, cp->year);
    sleep(1);
}

void *action1(lqueue_t *lqp) {
    lqapply(lqp, printCar);
    return NULL;
}

int main() {
    // single thread

    car_t *car_p = make_car("Honda Civic", 10000, 2018);
    car_t *car2_p = make_car("RB20", 30000, 2024);
    car_t *car3_p = make_car("Jeep Wrangler Sahara", 55000, 2024);


    lqueue_t *lqp = lqopen();
    lqput(lqp, car_p);
    lqput(lqp, car2_p);
    lqput(lqp, car3_p);

    pthread_t thread1;

    if (pthread_create(&thread1, NULL, action1, (void *)lqp) != 0) {
        perror("Failed to create thread 1");
        lqclose(lqp);
        exit(EXIT_FAILURE);
    }

    pthread_join(thread1, NULL);
    lqclose(lqp);
    
    exit(EXIT_SUCCESS);
}
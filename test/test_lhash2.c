/* 
 * test_lqueue2.c --- 
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
#include <lhash.h>
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

void *action1(lhashtable_t *lhtp) {
    // do something
    lhapply(lhtp, printCar);
    return NULL;
}

bool compareCar(car_t *cp, const char* plate) {
    if (cp == NULL) {return false;}
    return strcmp(cp->plate, plate) == 0;
}

void *action2(lhashtable_t *lhtp) {
    car_t *resultp = (car_t*)lhremove(lhtp, (bool (*)(void *, const void *))compareCar, "12345", 5);
    sleep(2);
    printf("The first car in queue is %s \n", resultp->plate);
    return NULL;
}

int main() {
    // dual thread
    lhashtable_t *lhtp = lhopen(10);

    car_t *car_p = make_car("12345", 10000, 2018);
    car_t *car2_p = make_car("67891", 30000, 2024);
    car_t *car3_p = make_car("11121", 55000, 2024);

    lhput(lhtp, car_p, car_p->plate, 5);
    lhput(lhtp, car2_p, car_p->plate, 5);
    lhput(lhtp, car3_p, car_p->plate, 5);

    pthread_t thread1, thread2;

    if (pthread_create(&thread1, NULL, action1, (void *)lhtp) != 0) {
        perror("Failed to create thread 1");
        lhclose(lhtp);
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&thread2, NULL, action2, (void *)lhtp) != 0) {
        perror("Failed to create thread 2");
        lhclose(lhtp);
        exit(EXIT_FAILURE);
    }

    pthread_join(thread2, NULL);
    pthread_join(thread1, NULL);

    free(car_p);
    free(car2_p);
    free(car3_p);

    exit(EXIT_SUCCESS);
    
}
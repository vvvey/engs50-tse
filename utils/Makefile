CFLAGS=-Wall -pedantic -std=c11 -g

OFILES=queue.o hash.o webpage.o pageio.o indexio.o lqueue.o lhash.o

all: $(OFILES)
	ar cr ../lib/libutils.a $(OFILES)
	
queue.o: queue.c queue.h
	gcc $(CFLAGS) -I./ -c queue.c

lqueue.o: lqueue.c lqueue.h
	gcc $(CFLAGS) -I./ -lpthread -c lqueue.c

hash.o: hash.c hash.h
	gcc $(CFLAGS) -I./ -c hash.c

webpage.o: webpage.c webpage.h
	gcc $(CFLAGS) -I./ -c webpage.c

pageio.o: pageio.c pageio.h
	gcc $(CFLAGS) -I./ -c pageio.c

indexio.o: indexio.c indexio.h
	gcc $(CFLAGS) -I./ -c indexio.c

lhash.o: lhash.c lhash.h
	gcc $(CFLAGS) -I./ -c lhash.c

clean:
	rm -f *.o

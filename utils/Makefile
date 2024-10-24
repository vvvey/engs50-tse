CFLAGS=-Wall -pedantic -std=c11 -g

OFILES=queue.o hash.o webpage.o pageio.o

# Generate Object Files
all: $(OFILES)
		ar cr ../lib/libutils.a $(OFILES)
	
queue.o:	queue.c queue.h
	gcc $(CFLAGS) -I./ -c queue.c

hash.o: hash.c hash.h
	gcc $(CFLAGS) -I./  -c hash.c

webpage.o: webpage.c webpage.h
	gcc $(CFLAGS) -I./ -c webpage.c

pageio.o: pageio.c pageio.h
	gcc $(CFLAGS) -I./ -c pageio.c

# Clean up generated object files and executables
clean:
	rm -f *.o

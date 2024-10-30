CC = gcc
CFLAGS = -Wall -Werror

all: display

display: display.o pointcloud.o util.o bmp.o
	$(CC) $(CFLAGS) -o display display.o pointcloud.o util.o bmp.o -lm

display.o: display.c pointcloud.h
	$(CC) $(CFLAGS) -c display.c -lm

pointcloud.o: pointcloud.c pointcloud.h util.h bmp.h
	$(CC) $(CFLAGS) -c pointcloud.c -lm

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c -lm

bmp.o: bmp.c bmp.h
	$(CC) $(CFLAGS) -c bmp.c -lm 

clean:
	rm -f *.o display
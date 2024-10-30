CC = gcc
CFLAGS = -Wall -g

# Main targets
watershed: watershed.o pointcloud.o util.o bmp.o
	$(CC) -o watershed watershed.o pointcloud.o util.o bmp.o -lm

display: display.o pointcloud.o util.o bmp.o
	$(CC) -o display display.o pointcloud.o util.o bmp.o -lm

test_pointcloud: test_pointcloud.o pointcloud.o util.o bmp.o
	$(CC) -o test_pointcloud test_pointcloud.o pointcloud.o util.o bmp.o -lm

# Object files
watershed.o: watershed.c pointcloud.h util.h
	$(CC) $(CFLAGS) -c watershed.c

display.o: display.c pointcloud.h util.h
	$(CC) $(CFLAGS) -c display.c

pointcloud.o: pointcloud.c pointcloud.h util.h
	$(CC) $(CFLAGS) -c pointcloud.c

util.o: util.c util.h
	$(CC) $(CFLAGS) -c util.c

bmp.o: bmp.c bmp.h
	$(CC) $(CFLAGS) -c bmp.c

test_pointcloud.o: test_pointcloud.c pointcloud.h
	$(CC) $(CFLAGS) -c test_pointcloud.c

# Test target
test: test_pointcloud
	./test_pointcloud

# Cleanup
clean:
	rm -f *.o watershed display test_pointcloud out.gif

.PHONY: clean test
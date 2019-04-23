# file
dict.o: dict.c dict.h heap.h
heap.o: heap.c heap.h
mock.o: mock.c mock.h
main.o: main.c mock.h heap.h dict.h

OBJ = main.o mock.o heap.o dict.o
PROGNAME = url_counter
CCOPT = -O2

build: $(OBJ)
	$(CC) -o $(PROGNAME) $(CCOPT) $(OBJ)

run:
	./${PROGNAME}

server:
	build run

clean:
	rm -rf $(PROGNAME) *.o
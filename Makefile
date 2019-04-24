CFLAGS=-std=c99

# file
dict.o: dict.c dict.h heap.h
heap.o: heap.c heap.h
main.o: main.c heap.h dict.h
mock.o: mock.c

OBJ = main.o heap.o dict.o
MOCK = mock.o

CCOPT = -O2

build: $(OBJ) $(MOCK)
	$(CC) -o url_counter $(CCOPT) $(OBJ)
	$(CC) -o mock $(CCOPT) $(MOCK)

clean:
	rm -rf url_counter mock
	rm -rf *.o out

test: build
	mkdir -p out
	./mock out/url 1024
	./url_counter out/url

#内存泄露检测用的是valgrind --tool=memcheck --leak-check=full ./${PROGNAME} out/url


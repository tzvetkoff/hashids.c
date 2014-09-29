all: main test

main: main.o hashids.o
	$(CC) -o hashids main.o hashids.o -lm

test: test.o hashids.o
	$(CC) -o test test.o hashids.o -lm

main.o: main.c hashids.o
	$(CC) -c -o main.o main.c

test.o: test.c hashids.o
	$(CC) -c -o test.o test.c

hashids.o: hashids.c hashids.h
	$(CC) -c -o hashids.o hashids.c

clean:
	$(RM) hashids test *.o

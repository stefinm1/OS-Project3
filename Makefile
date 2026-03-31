all:
	gcc -c *.c
	gcc -o main main.o 
clean:
	rm -f main *.o


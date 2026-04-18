all:
	gcc -c *.c
	gcc -o main1 main1.o
	gcc -o main2 main2.o
	gcc -o main3 main3.o

clean:
	rm -f main1 main2 main3 *.o
	rm out1.txt
	rm out2.txt
	rm out3.txt
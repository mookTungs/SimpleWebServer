CC = gcc

sws: sws.o
	$(CC) -o sws sws.o
	
clean:
	rm -f sws sws.o
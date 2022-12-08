CC=gcc
FLAGS= -Wall -g

all: send recv

SendData.o: SendData.c
	$(CC) $(FLAGS) -c SendData.c

send: SendData.o
	$(CC) $(FLAGS) -o send SendData.o 

recvData.o: recvData.c
	$(CC) $(FLAGS) -c recvData.c

recv: recvData.o
	$(CC) $(FLAGS) -o recv recvData.o 

.PHONY: clean all 

clean:
	rm -f *.o *.a *.so send recv

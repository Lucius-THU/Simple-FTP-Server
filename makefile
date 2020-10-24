all: server

server: *.c *.h
	gcc -o server *.c -lpthread -Wall
    
clean:
	rm -rf server
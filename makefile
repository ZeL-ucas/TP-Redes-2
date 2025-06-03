all: 
	mkdir -p bin
	mkdir -p obj
	gcc -Wall -c functions/connectionFunctions.c -o obj/connectionFunctions.o
	gcc -Wall client.c obj/connectionFunctions.o -o bin/client
	gcc -Wall server.c obj/connectionFunctions.o -o bin/server

clean:
	rm -rf bin/
	rm -rf obj/
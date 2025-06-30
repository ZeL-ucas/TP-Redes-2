all: 
		mkdir -p bin
		mkdir -p src/obj
		gcc -Wall -c src/functions/connectionFunctions.c -o src/obj/connectionFunctions.o
		gcc -Wall -c src/functions/gameFunctions.c -o src/obj/gameFunctions.o
		gcc -Wall src/client.c src/obj/connectionFunctions.o -o bin/client -lpthread
		gcc -Wall src/server.c src/obj/connectionFunctions.o src/obj/gameFunctions.o -o bin/server -lpthread -lm
clean:
	rm -rf bin/
	rm -rf src/obj/

run: all
	gnome-terminal -- bash -c "./bin/server v6 51500; exec bash"
	gnome-terminal -- bash -c "./bin/client ::0 51500 -nick zé; exec bash"

clang:
	clang-format -i ./src/client.c
	clang-format -i ./src/server.c
	clang-format -i ./src/functions/connectionFunctions.c
	clang-format -i ./src/functions/gameFunctions.c

run3: all
	gnome-terminal -- bash -c "./bin/server v6 51500; exec bash"
	gnome-terminal -- bash -c "./bin/client ::0 51500 -nick zé; exec bash"
	gnome-terminal -- bash -c "./bin/client ::0 51500 -nick zes; exec bash"
	gnome-terminal -- bash -c "./bin/client ::0 51500 -nick paulo; exec bash"
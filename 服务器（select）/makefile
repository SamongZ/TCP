all: server client

server : server.o
	g++ -g -o server server.o
client : client.o
	g++ -g -o client client.o

server.o : server.cpp
	g++ -g -c server.cpp -o server.o
client.o : client.cpp
	g++ -g -c client.cpp -o client.o

clear:
	rm -rf *.o server client

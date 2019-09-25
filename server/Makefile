

server : server.o thread.o socket.o socketserver.o Blockable.o
	g++ -o server server.o thread.o socket.o socketserver.o Blockable.o -pthread -l rt


ServerReference : ServerReference.o thread.o socket.o socketserver.o Blockable.o
	g++ -o ServerReference ServerReference.o thread.o socket.o socketserver.o Blockable.o -pthread -l rt

Blockable.o : Blockable.h Blockable.cpp
	g++ -c Blockable.cpp -std=c++11

server.o : server.cpp thread.h socketserver.h Game.h SharedStuff.h
	g++ -c server.cpp -std=c++11

ServerReference.o : ServerReference.cpp thread.h socketserver.h
	g++ -c ServerReference.cpp -std=c++11

thread.o : thread.cpp thread.h
	g++ -c thread.cpp -std=c++11

socket.o : socket.cpp socket.h
	g++ -c socket.cpp -std=c++11

socketserver.o : socketserver.cpp socket.h socketserver.h
	g++ -c socketserver.cpp -std=c++11

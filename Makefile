
all: build
	./server

# Link
build:
	g++ -std=c++11 -Wall -o server Socket.cpp HTTPServer.cpp main.cpp

clean:
	rm -f *.o server

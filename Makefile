main: prep
	g++ -lncurses -std=gnu++17 -lpthread main.cpp -o bin/curseranger

prep:
	mkdir -p bin

clean:
	rm -rf bin

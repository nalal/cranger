main: prep
	g++ -lncurses -std=gnu++17 -lpthread main.cpp -o bin/cranger

prep:
	mkdir -p bin

install: main
	mkdir -p ~/.local/bin
	cp bin/*  ~/.local/bin

clean:
	rm -rf bin

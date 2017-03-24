all:
	mkdir -p bin
	g++ main.cpp -o bin/rback
run:
	clear
	bin/rback
clean:
	rm -r bin/
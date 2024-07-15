all:
	mkdir build
	gcc -g -c json.c -lm -o build/libjson.o
	ar cr build/libjsonlib.a build/libjson.o
	gcc -g main.c -Lbuild -ljsonlib -lcurl -lm -Wall -Wextra -Wpedantic -Werror -o main -lbsd
clean:
	rm -rf build

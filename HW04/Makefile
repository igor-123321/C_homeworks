all:
	mkdir build
	gcc -c json.c -lm -o build/libjson.o
	ar cr build/libjsonlib.a build/libjson.o
	gcc main.c -Lbuild -ljsonlib -lcurl -lm -Wall -Wextra -Wpedantic -Werror -o main -lbsd
clean:
	rm -rf build

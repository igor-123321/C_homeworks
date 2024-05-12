all:
	gcc -c json.c -lm -o libjson.o
	ar cr libjsonlib.a libjson.o
	gcc main.c -L. -ljsonlib -lcurl -lm -o main
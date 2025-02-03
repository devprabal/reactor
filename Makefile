all:
	gcc --std=c11 -Werror -Wall -pedantic -lpthread main.c
valgrind:
	valgrind --leak-check=full --show-leak-kinds=all --quiet ./a.out
clean:
	rm -rf a.out

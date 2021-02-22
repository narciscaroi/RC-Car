all: run

build: server.c
	gcc -Wall -pthread -o server server.c -lpigpio -lrt -lm

run: build
	sudo ./server

reset:
	gpio write 6 0
	gpio write 26 0
	gpio write 13 0
	gpio write 19 0

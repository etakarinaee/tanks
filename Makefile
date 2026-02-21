
CC = gcc 
CFLAGS = -Wall -Wextra -std=c89 -I.
EXENAME = tanks

all:
	$(CC) $(CFLAGS) main.c glad.c renderer.c -o $(EXENAME) -lglfw -lGL

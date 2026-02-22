CFLAGS = -Wall -Wextra -std=c89 -I.

OBJ = main.o glad.o renderer.o

tanks: $(OBJ)
	$(CC) $(OBJ) -o $@ -lglfw -lGL

run: tanks
	./tanks

clean:
	rm -f $(OBJ) tanks

.PHONY: run clean
CC=clang
DEPS=glfw3 opengl glew
CFLAGS=-Wall -Wextra -std=c11 -pedantic `pkg-config --cflags $(DEPS)` -ggdb
LIBS=`pkg-config --libs $(DEPS)` -lm
SRC=main.c renderer.c

.PHONY: app

app: $(SRC)
	$(CC) $(CFLAGS) -o app $(SRC) $(LIBS)

run: app
	./$<

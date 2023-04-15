CC=clang
DEPS=glfw3 opengl glew freetype2
CFLAGS=-Wall -Wextra -std=c11 -pedantic `pkg-config --cflags $(DEPS)` -ggdb
LIBS=`pkg-config --libs $(DEPS)` -lm
SRC=src/main.c src/renderer.c src/glyph.c

.PHONY: app

app: $(SRC)
	$(CC) $(CFLAGS) -o app $(SRC) $(LIBS)

run: app
	./$<

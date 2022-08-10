CC=gcc 
CFLAGS=-std=c99 -Wall -Wextra -g
LINK=-L./external/glfw/build/src
INCLUDE=-I./external/glfw/include -I/usr/include/freetype2 -I/usr/include/libpng16 

main: main.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LINK) -o opengl_text_demo main.c -lGL -lglfw3 -ldl -lm -lpthread -lfreetype

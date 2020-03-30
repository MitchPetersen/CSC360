CC = gcc
CFLAGS = -g -Wall -Wno-deprecated-declarations -Werror

all: file

file: File.c bitmap.c bitmap.h disk_controller.c disk_controller.h
	$(CC) $(CFLAGS) -o File File.c bitmap.c disk_controller.c -lncurses



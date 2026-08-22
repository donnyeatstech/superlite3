CC=gcc
CFLAGS=-Wall -Wextra


build: src/main.c src/os/os.c
	$(CC) $(CFLAGS) -o superlite src/main.c src/os/os.c

debug: src/main.c src/os/os.c
	$(CC) $(CFLAGS) -o superlite src/main.c src/os/os.c  -g

clean: 
	rm superlite

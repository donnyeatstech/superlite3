CC=gcc
CFLAGS=-Wall -Wextra


build: src/main.c src/os/os.c src/os/backend.h src/os/real_backend.c
	$(CC) $(CFLAGS) -o superlite src/main.c src/os/os.c src/os/real_backend.c

debug: src/main.c src/os/os.c src/os/backend.h src/os/real_backend.c
	$(CC) $(CFLAGS) -o superlite src/main.c src/os/os.c src/os/real_backend.c -g

test: tests/test_main.c src/os/os.c src/os/backend.h src/os/real_backend.c tests/os/test_backend.c
	$(CC) $(CFLAGS) -o superlite tests/test_main.c src/os/os.c src/os/real_backend.c tests/os/test_backend.c

clean: 
	rm superlite

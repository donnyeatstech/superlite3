CC=gcc


build: src/main.c src/os/os.c
	$(CC) -o superlite src/main.c src/os/os.c 

debug: src/main.c src/os/os.c
	$(CC) -o superlite src/main.c src/os/os.c  -g

clean: 
	rm superlite

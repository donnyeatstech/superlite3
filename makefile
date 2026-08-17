CC=gcc


build: src/main.c src/pager/pager.c
	$(CC) -o superlite src/main.c src/pager/pager.c 


clean: 
	rm superlite

linuxdemo: linuxdemo.c
	gcc -Wall -Wextra -O3 -o linuxdemo linuxdemo.c -lSDL2 -lm
	strip linuxdemo

clean:
	rm -f linuxdemo
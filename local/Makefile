main: main.c
	gcc -g -Wall main.c initialize.c start.c

daemon: splinterDaemon.c
	gcc -o splintered -Wall splinterDaemon.c

clean:
	rm -rf .splinter/ a.out splintered

 # Creates binary for server.c
 all:cserver.c 
	gcc -g -Wall -o server server.c

 clean: 
	rm server

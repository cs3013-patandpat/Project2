all: getloginuid shift2user

getloginuid: getloginuid.c
	gcc -o getloginuid getloginuid.c

shift2user: shift2user.c
	gcc -o shift2user shift2user.c

clean:
	-rm getloginuid
	-rm shift2user

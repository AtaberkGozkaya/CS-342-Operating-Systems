all: pwc twc
pwc: pwc.c
	gcc -Wall -o pwc pwc.c -lrt
twc: twc.c
	gcc -Wall -o twc twc.c -lpthread
clean:
	rm -fr *~ pwc twc
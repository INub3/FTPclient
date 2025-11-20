# Makefile para cliente FTP 

CLTOBJ= TCPftp.o sources/connectsock.o sources/connectTCP.o sources/passivesock.o sources/passiveTCP.o sources/errexit.o

all: TCPftp 

TCPftp:	${CLTOBJ}
	cc -o TCPftp ${CLTOBJ}

clean:
	rm $(CLTOBJ) 

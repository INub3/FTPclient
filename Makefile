# Makefile para cliente FTP 

CLTOBJ= TCPftp.o sources/connectsock.o sources/connectTCP.o sources/passivesock.o sources/passiveTCP.o sources/errexit.o
FTPOBJ= FTPsessions.o sources/connectsock.o sources/connectTCP.o sources/passivesock.o sources/passiveTCP.o sources/errexit.o

all: TCPftp FTPsessions

TCPftp: ${CLTOBJ}
	cc -o TCPftp ${CLTOBJ}

FTPsessions: ${FTPOBJ}
	cc -o FTPsessions ${FTPOBJ}

clean:
	rm -f $(CLTOBJ) $(FTPOBJ) TCPftp FTPsessions

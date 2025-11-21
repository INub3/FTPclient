# Makefile para cliente FTP 

CLTOBJ= EnriquezM-clienteFTP.o sources/connectsock.o sources/connectTCP.o sources/passivesock.o sources/passiveTCP.o sources/errexit.o
FTPOBJ= EnriquezM-FTPsessions.o sources/connectsock.o sources/connectTCP.o sources/passivesock.o sources/passiveTCP.o sources/errexit.o

all: EnriquezM-clienteFTP EnriquezM-FTPsessions

EnriquezM-clienteFTP: ${CLTOBJ}
	cc -o EnriquezM-clienteFTP ${CLTOBJ}

EnriquezM-FTPsessions: ${FTPOBJ}
	cc -o EnriquezM-FTPsessions ${FTPOBJ}

clean:
	rm -f $(CLTOBJ) $(FTPOBJ) EnriquezM-clienteFTP EnriquezM-FTPsessions
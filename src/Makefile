# Source names
RCV = icmp_receive
SND = icmp_send
SUM = icmp_checksum
TRC = icmp_traceroute

# Compiler settings
CC = gcc

CFLAGS = -g -std=gnu99 -Wall -Wextra

# Make procedures
make:
	${CC} -c ${CFLAGS} ${RCV}.c -o ${RCV}.o 
	${CC} -c ${CFLAGS} ${SND}.c -o ${SND}.o 	
	${CC} -c ${CFLAGS} ${SUM}.c -o ${SUM}.o 	
	${CC} -c ${CFLAGS} ${TRC}.c -o ${TRC}.o 	
	${CC} ${CFLAGS} -o traceroute ${RCV}.o ${SND}.o ${SUM}.o ${TRC}.o	

clean:
	-rm -f *.o

distclean:
	-rm -f *.o traceroute

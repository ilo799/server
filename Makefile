###############################################################################
#
# File:         Makefile
# RCS:          $Id: Makefile,v 1.1 2009/10/09 04:38:08 npb853 Exp $
# Description:  Guess
# Author:       Fabian E. Bustamante
#               Northwestern Systems Research Group
#               Department of Computer Science
#               Northwestern University
# Created:      Fri Sep 12, 2003 at 15:56:30
# Modified:     Wed Sep 24, 2003 at 18:31:43 fabianb@cs.northwestern.edu
# Language:     Makefile
# Package:      N/A
# Status:       Experimental (Do Not Distribute)
#
# (C) Copyright 2003, Northwestern University, all rights reserved.
#
###############################################################################

# handin info
TEAM = `whoami`
VERSION = `date +%Y%m%d%H%M%S`
PROJ = http_server

CC = gcc
MV = mv
CP = cp
RM = rm
MKDIR = mkdir
TAR = tar cvf
COMPRESS = gzip
#CFLAGS = -g -Wall -D HAVE_CONFIG_H
CFLAGS = -g -Wall -O2 -D HAVE_CONFIG_H

DELIVERY = Makefile *.h *.c
PROGS = http_server
SRCS = http_server.c thread_pool.c util.c seats.c
OBJS = ${SRCS:.c=.o}

all: ${PROGS}

#test-reg: handin
#	HANDIN=`pwd`/${TEAM}-${VERSION}-${PROJ}.tar.gz;\
#	cd testsuite;\
#	bash ./run_testcase.sh $${HANDIN};

handin: cleanAll
	${TAR} ${TEAM}-${VERSION}-${PROJ}.tar ${DELIVERY}
	${COMPRESS} ${TEAM}-${VERSION}-${PROJ}.tar

.o:
	${CC} *.c  *.h

http_server: ${OBJS}
	${CC} ${OBJS} -o $@  -lpthread

clean:
	${RM} -f *.o *~ *.h.gch

cleanAll: clean
	${RM} -f ${PROGS} ${TEAM}-${VERSION}-${PROJ}.tar.gz

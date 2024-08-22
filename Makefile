VERSION = 0.0
NAME = jpwm

PREFIX ?= /usr/local
DESTDIR ?=

INCS = -I.
LIBS = -lxcb

CC ?= gcc
CFLAGS ?= -std=c99 -pedantic -Wall -Wextra -Og ${INCS} \
	  -D_POSIX_C_SOURCE -D_XOPEN_SOURCE=700L -DVERSION=\"${VERSION}\" -DDEBUG -g
LDFLAGS ?= ${LIBS}

SRC = ${NAME}.c
OBJ = ${SRC:.c=.o}

all: ${NAME}

.c.o:
	${CC} -c ${CFLAGS} $< -o $@

${OBJ}: config.h

config.h:
	cp config.def.h $@

${NAME}: ${OBJ}
	${CC} ${OBJ} -o $@ ${LDFLAGS}

clean:
	rm -f ${OBJ} ${NAME}

install: all
	install -Dm755 ${NAME} ${DESTDIR}${PREFIX}/bin/${NAME}

uninstall:
	rm -fv ${DESTDIR}${PREFIX}/bin/${NAME}

.PHONY: all clean install uninstall

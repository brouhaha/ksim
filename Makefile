# ksim: an 8080 simulator
# Copyright 2011, 2014 Eric Smith <spacewar@gmail.com>
# SPDX-License-Identifier: GPL-3.0

CFLAGS = -Wall -Wextra -Werror -g
LDFLAGS = -g

all: ksim

clean:
	rm ksim.o ksim

ksim: ksim.o bdos.o

ksim.o: ksim.c ksim.h bdos.h

bdos.o: bdos.c ksim.h bdos.h 

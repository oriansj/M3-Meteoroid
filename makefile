## Copyright (C) 2020 Jeremiah Orians
## This file is part of M3-Meteoroid.
##
## M3-Meteoroid is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## M3-Meteoroid is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with M3-Meteoroid.  If not, see <http://www.gnu.org/licenses/>.

# Prevent rebuilding
VPATH = bin:test:test/results

# C compiler settings
CC?=gcc
CFLAGS:=$(CFLAGS) -D_GNU_SOURCE -O0 -std=c99 -ggdb

all: M3-Meteoroid-x86

M3-Meteoroid-x86: interface.c x86.c Meteoroid.c Meteoroid.h endian.c debug.c functions/require.c functions/file_print.c functions/raw_write.c functions/match.c functions/numerate.c functions/in_set.c | bin
	$(CC) $(CFLAGS) interface.c x86.c Meteoroid.c endian.c debug.c functions/require.c functions/file_print.c functions/raw_write.c functions/match.c functions/numerate.c functions/in_set.c -o bin/M3-Meteoroid-x86

# Clean up after ourselves
.PHONY: clean
clean:
	rm -rf bin/

# Directories
bin:
	mkdir -p bin

results:
	mkdir -p test/results

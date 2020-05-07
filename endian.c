/* Copyright (C) 2020 Jeremiah Orians
 * This file is part of M3-Meteoroid.
 *
 * M3-Meteoroid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * M3-Meteoroid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with M3-Meteoroid.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Meteoroid.h"

int read_half_little_endian(FILE* f, char* failure)
{
	int a = 0;
	int b = 0;
	int r = 0;

	/* Read first byte */
	int c = fgetc(f);
	require(EOF != c, failure);
	b = c;

	/* Read second byte */
	c = fgetc(f);
	require(EOF != c, failure);
	a = c;

	/* Use multiplication to ensure platform correct behavior */
	r = (a * 256) + b;
	return r;
}

int read_half_big_endian(FILE* f, char* failure)
{
	int r = 0;

	/* Read first byte */
	int c = fgetc(f);
	require(EOF != c, failure);
	r = c * 256;

	/* Read second byte */
	c = fgetc(f);
	require(EOF != c, failure);
	r = r + c;

	return r;
}

int read_word_little_endian(FILE* f, char* failure)
{
	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;
	int r = 0;

	/* Read first byte */
	int t = fgetc(f);
	require(EOF != t, failure);
	d = t;

	/* Read second byte */
	t = fgetc(f);
	require(EOF != t, failure);
	c = t;

	/* Read third byte */
	t = fgetc(f);
	require(EOF != t, failure);
	b = t;

	/* Read fourth byte */
	t = fgetc(f);
	require(EOF != t, failure);
	a = t;

	/* Use multiplication to ensure platform correct behavior */
	r = (a * 16777216) + (b * 65536) + (c * 256) + d;
	return r;
}

int read_word_big_endian(FILE* f, char* failure)
{
	int r = 0;
	int c;
	int i;

	/* Read 4 bytes */
	for(i = 4; i > 0; i = i - 1)
	{
		c = fgetc(f);
		require(EOF != c, failure);
		r = (r * 256) + c;
	}

	return r;
}

int read_double_little_endian(FILE* f, char* failure)
{
	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;
	int r = 0;
	int i;

	/* Read first byte */
	int t = fgetc(f);
	require(EOF != t, failure);
	d = t;

	/* Read second byte */
	t = fgetc(f);
	require(EOF != t, failure);
	c = t;

	/* Read third byte */
	t = fgetc(f);
	require(EOF != t, failure);
	b = t;

	/* Read fourth byte */
	t = fgetc(f);
	require(EOF != t, failure);
	a = t;

	/* Use multiplication to ensure platform correct behavior */
	r = (a * 16777216) + (b * 65536) + (c * 256) + d;

	/* make sure it fits in 32bits */
	for(i = 4; i > 0; i = i - 1)
	{
		c = fgetc(f);
		require(EOF != c, failure);
		require(0 == c, "M3-Meteoroid currently only supports binaries under 2GB in size\n");
	}

	return r;
}

int read_double_big_endian(FILE* f, char* failure)
{
	int r = 0;
	int c;
	int i;

	/* make sure it fits in 32bits */
	for(i = 4; i > 0; i = i - 1)
	{
		c = fgetc(f);
		require(EOF != c, failure);
		require(0 == c, "M3-Meteoroid currently only supports binaries under 2GB in size\n");
	}

	/* Read 4 bytes */
	for(i = 4; i > 0; i = i - 1)
	{
		c = fgetc(f);
		require(EOF != c, failure);
		r = (r * 256) + c;
	}

	return r;
}

int read_half(FILE* f, char* failure)
{
	if(BigEndian) return read_half_big_endian(f, failure);
	else return read_half_little_endian(f, failure);
}

int read_word(FILE* f, char* failure)
{
	if(BigEndian) return read_word_big_endian(f, failure);
	else return read_word_little_endian(f, failure);
}

int read_double(FILE* f, char* failure)
{
	if(BigEndian) return read_double_big_endian(f, failure);
	else return read_double_little_endian(f, failure);
}

SCM read_register(FILE* f, char* failure)
{
	if(largeint) return read_double(f, failure);
	else return read_word(f, failure);
}

char* read_string(FILE* f, int base, int offset, char* error)
{
	/* Deal with NULL case */
	if(0 == offset) return "";

	/* Protect pointer to file */
	fflush(f);
	SCM p = ftell(f);

	char* r = calloc(MAX_STRING, sizeof(char));
	fseek(f, base + offset, SEEK_SET);
	int c = fgetc(f);
	int i = 0;
	while(0 != c)
	{
		require(EOF != c, error);
		r[i] = c;
		i = i + 1;
		c = fgetc(f);
	}

	/* Return file pointer to previous place */
	fseek(f, p, SEEK_SET);
	return r;
}

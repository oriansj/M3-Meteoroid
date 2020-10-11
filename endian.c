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

int get_char(struct segment* f)
{
	if(read_offset > f->size) return -1;

	/* GCC seems to think returning more than a byte from a byte string is a good idea sometimes ??? */
	int r = (f->contents[read_offset]) & 0xFF;
	read_offset = read_offset + 1;
	return r;
}

void put_char(int c, struct segment* f)
{
	f->contents[write_offset] = c;
	write_offset = write_offset + 1;
}

int read_half_little_endian(struct segment* f, char* failure)
{
	int a = 0;
	int b = 0;
	int r = 0;

	/* Read first byte */
	int c = get_char(f);
	require(EOF != c, failure);
	b = c;

	/* Read second byte */
	c = get_char(f);
	require(EOF != c, failure);
	a = c;

	/* Use multiplication to ensure platform correct behavior */
	r = (a * 256) + b;
	return r;
}

int read_half_big_endian(struct segment* f, char* failure)
{
	int r = 0;

	/* Read first byte */
	int c = get_char(f);
	require(EOF != c, failure);
	r = c * 256;

	/* Read second byte */
	c = get_char(f);
	require(EOF != c, failure);
	r = r + c;

	return r;
}

int read_word_little_endian(struct segment* f, char* failure)
{
	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;
	int r = 0;

	/* Read first byte */
	int t = get_char(f);
	require(EOF != t, failure);
	d = t;

	/* Read second byte */
	t = get_char(f);
	require(EOF != t, failure);
	c = t;

	/* Read third byte */
	t = get_char(f);
	require(EOF != t, failure);
	b = t;

	/* Read fourth byte */
	t = get_char(f);
	require(EOF != t, failure);
	a = t;

	/* Use multiplication to ensure platform correct behavior */
	r = (a * 16777216) + (b * 65536) + (c * 256) + d;
	return r;
}

int read_word_big_endian(struct segment* f, char* failure)
{
	int r = 0;
	int c;
	int i;

	/* Read 4 bytes */
	for(i = 4; i > 0; i = i - 1)
	{
		c = get_char(f);
		require(EOF != c, failure);
		r = (r * 256) + c;
	}

	return r;
}

int read_double_little_endian(struct segment* f, char* failure)
{
	int a = 0;
	int b = 0;
	int c = 0;
	int d = 0;
	int r = 0;
	int i;

	/* Read first byte */
	int t = get_char(f);
	require(EOF != t, failure);
	d = t;

	/* Read second byte */
	t = get_char(f);
	require(EOF != t, failure);
	c = t;

	/* Read third byte */
	t = get_char(f);
	require(EOF != t, failure);
	b = t;

	/* Read fourth byte */
	t = get_char(f);
	require(EOF != t, failure);
	a = t;

	/* Use multiplication to ensure platform correct behavior */
	r = (a * 16777216) + (b * 65536) + (c * 256) + d;

	/* make sure it fits in 32bits */
	for(i = 4; i > 0; i = i - 1)
	{
		c = get_char(f);
		require(EOF != c, failure);
		require(0 == c, "M3-Meteoroid currently only supports binaries under 2GB in size\n");
	}

	return r;
}

int read_double_big_endian(struct segment* f, char* failure)
{
	int r = 0;
	int c;
	int i;

	/* make sure it fits in 32bits */
	for(i = 4; i > 0; i = i - 1)
	{
		c = get_char(f);
		require(EOF != c, failure);
		require(0 == c, "M3-Meteoroid currently only supports binaries under 2GB in size\n");
	}

	/* Read 4 bytes */
	for(i = 4; i > 0; i = i - 1)
	{
		c = get_char(f);
		require(EOF != c, failure);
		r = (r * 256) + c;
	}

	return r;
}

int read_half(struct segment* f, char* failure)
{
	if(BigEndian) return read_half_big_endian(f, failure);
	else return read_half_little_endian(f, failure);
}

int read_word(struct segment* f, char* failure)
{
	if(BigEndian) return read_word_big_endian(f, failure);
	else return read_word_little_endian(f, failure);
}

int read_double(struct segment* f, char* failure)
{
	if(BigEndian) return read_double_big_endian(f, failure);
	else return read_double_little_endian(f, failure);
}

SCM read_register(struct segment* f, char* failure)
{
	if(largeint) return read_double(f, failure);
	else return read_word(f, failure);
}

char* read_string(struct segment* f, int base, int offset, char* error)
{
	/* Deal with NULL case */
	if(0 == offset) return "";

	/* Protect pointer to file */
	SCM p = read_offset;

	char* r = calloc(MAX_STRING, sizeof(char));
	read_offset = base + offset;
	int c = get_char(f);
	int i = 0;
	while(0 != c)
	{
		require(EOF != c, error);
		r[i] = c;
		i = i + 1;
		c = get_char(f);
	}

	/* Return file pointer to previous place */
	read_offset = p;
	return r;
}

void write_half_little_endian(struct segment* f, int o)
{
	int high = 0xFF & (o >> 8);
	int low = o & 0xFF;
	put_char(low, f);
	put_char(high, f);
}

void write_half_big_endian(struct segment* f, int o)
{
	int high = 0xFF & (o >> 8);
	int low = o & 0xFF;
	put_char(high, f);
	put_char(low, f);
}

void write_word_little_endian(struct segment* f, int o)
{
	int high = 0xFFFF & (o >> 16);
	int low = o & 0xFFFF;
	write_half_little_endian(f, low);
	write_half_little_endian(f, high);
}

void write_word_big_endian(struct segment* f, int o)
{
	int high = 0xFFFF & (o >> 16);
	int low = o & 0xFFFF;
	write_half_big_endian(f, high);
	write_half_big_endian(f, low);
}

void write_double_little_endian(struct segment* f, int o)
{
	/* currently only support values that fit in 32 bits */
	write_half_little_endian(f, o);
	write_half_little_endian(f, 0);
}

void write_double_big_endian(struct segment* f, int o)
{
	/* currently only support values that fit in 32 bits */
	write_half_big_endian(f, 0);
	write_half_big_endian(f, o);
}

void write_half(struct segment* f, int o)
{
	if(BigEndian) write_half_big_endian(f, o);
	else write_half_little_endian(f, o);
}

void write_word(struct segment* f, int o)
{
	if(BigEndian) write_word_big_endian(f, o);
	else write_word_little_endian(f, o);
}

void write_double(struct segment* f, int o)
{
	if(BigEndian) write_double_big_endian(f, o);
	else write_double_little_endian(f, o);
}

void write_register(struct segment* f, int o)
{
	if(largeint) write_double(f, o);
	else write_word(f, o);
}

void print_byte(int c, FILE* f)
{
	char* table = "0123456789ABCDEF";
	fputc(table[(c & 0xF0) >> 4], f);
	fputc(table[c & 0xF], f);
}

void print_address(SCM address, FILE* f)
{
	if(largeint)
	{
		require( 0 == address >> 32, "M3-Meteoroid currently only supports printing addresses for binaries under 2GB in size\n");
		print_byte(0, f);
		print_byte(0, f);
		print_byte(0, f);
		print_byte(0, f);
	}

	print_byte((address & 0xFF000000) >> 24, f);
	print_byte((address & 0xFF0000) >> 16, f);
	print_byte((address & 0xFF00) >> 8, f);
	print_byte((address & 0xFF), f);
}

struct segment* get_file(FILE* f, char* name)
{
	if(NULL == f)
	{
		file_print("Unable to open file ", stderr);
		file_print(name, stderr);
		file_print(" for reading\nExiting before problems occur\n", stderr);
		exit(EXIT_FAILURE);
	}

	read_offset = 0;
	fseek(f, 0, SEEK_END);
	fflush(f);
	struct segment* b = calloc(1, sizeof(struct segment));
	b->size = ftell(f);
	b->name = name;
	fseek(f, 0, SEEK_SET);
	b->contents = calloc(b->size + 4, sizeof(char));

	int i = 0;
	int C;
	while(i < b->size)
	{
		C = fgetc(f);
		require(EOF != C, "File changed size before done reading file\n");
		b->contents[i] = C;
		i = i + 1;
	}

	return b;
}

struct segment* output_buffer_generate()
{
	struct segment* r = calloc(1, sizeof(struct segment));
	if(largeint)
	{
		/* ELF header required */
		r->size = 64;
		/* .text segment entry */
		r->size = r->size + 52;
		/* .data segment entry */
		r->size = r->size + 52;
	}
	else
	{
		/* ELF header required */
		r->size = 52;
		/* .text segment entry */
		r->size = r->size + 32;
		/* .data segment entry */
		r->size = r->size + 32;
	}

	/* .text segment */
	r->size = r->size + text_size;
	/* .data segment */
	r->size = r->size + data_size;

	r->contents = calloc(r->size, sizeof(char));
	return r;
}

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

void print_byte(int c, FILE* f);
void print_address(SCM address, FILE* f);
int in_set(int c, char* s);

void sane_print(int c, FILE* f)
{
	if(in_set(c, " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~")) fputc(c, f);
	else
	{
		file_print("\\x", f);
		print_byte(c, f);
	}
}

void print_segment(struct elf_section_header* s, char* name)
{
	if(NULL != s)
	{
		require(match(s->sh_name, name), "Tried to print the wrong segment\nAborting\n");

		struct segment* contents = s->contents;
		file_print("Segment type: ", stdout);
		file_print(name, stdout);
		file_print("\n", stdout);

		int i = 0;
		int address = contents->starting_address;
		int size = s->sh_size;
		while(i < size)
		{
			print_address(address, stdout);
			file_print(":\t", stdout);
			print_byte(contents->contents[i], stdout);
			print_byte(contents->contents[i+1], stdout);
			print_byte(contents->contents[i+2], stdout);
			print_byte(contents->contents[i+3], stdout);

			file_print(" :: ", stdout);
			sane_print(contents->contents[i], stdout);
			sane_print(contents->contents[i+1], stdout);
			sane_print(contents->contents[i+2], stdout);
			sane_print(contents->contents[i+3], stdout);
			i = i + 4;
			address = address + 4;
			file_print("\n", stdout);
		}
	}
	else
	{
		file_print("EMPTY SEGMENT!\n", stdout);
		exit(EXIT_FAILURE);
	}
}

void print_file(struct elf_object_file* f)
{
	while(NULL != f)
	{
		file_print("FILE NAME: ", stdout);
		file_print(f->name, stdout);
		file_print("\n", stdout);

		if(NULL != f->text) print_segment(f->text, ".text");
		if(NULL != f->data) print_segment(f->data, ".data");

		f = f->next;
	}
}

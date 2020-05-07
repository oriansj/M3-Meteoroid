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

struct elf_header* read_elf_header(FILE* f);
struct program_header* read_program_header(FILE* f, struct elf_header* e);
struct section_header* read_section_header(FILE* f, struct elf_header* e);
struct symbol* read_symbols(FILE* f);
struct relocation* read_relocation(FILE* f, char* segment);
struct adjusted_relocation* read_adjusted_relocations(FILE* f, char* segment);
struct segment* read_segment(FILE* f, int offset, int size, char* name);

int main(int argc, char** argv)
{
	FILE* in;
	FILE* destination_file;

	int i = 1;
	while(i <= argc)
	{
		if(NULL == argv[i])
		{
			i = i + 1;
		}
		else if(match(argv[i], "-f") || match(argv[i], "--file"))
		{
			char* name = argv[i + 1];
			in = fopen(name, "r");
			if(NULL == in)
			{
				file_print("Unable to open for reading file: ", stderr);
				file_print(name, stderr);
				file_print("\n Aborting to avoid problems\n", stderr);
				exit(EXIT_FAILURE);
			}
			i = i + 2;
		}
		else if(match(argv[i], "-o") || match(argv[i], "--output"))
		{
			destination_file = fopen(argv[i + 1], "w");
			if(NULL == destination_file)
			{
				file_print("Unable to open for writing file: ", stderr);
				file_print(argv[i + 1], stderr);
				file_print("\n Aborting to avoid problems\n", stderr);
				exit(EXIT_FAILURE);
			}
			i = i + 2;
		}
		else if(match(argv[i], "-h") || match(argv[i], "--help"))
		{
			file_print(" -f input file\n -o output file\n --help for this message\n --version for file version\n", stdout);
			exit(EXIT_SUCCESS);
		}
		else if(match(argv[i], "-V") || match(argv[i], "--version"))
		{
			file_print("M3-Meteoroid-x86 v0.0\n", stderr);
			exit(EXIT_SUCCESS);
		}
		else
		{
			file_print("UNKNOWN ARGUMENT\n", stdout);
			exit(EXIT_FAILURE);
		}
	}

	struct elf_header* h = read_elf_header(in);
	struct program_header* p = read_program_header(in, h);
	struct section_header* s = read_section_header(in, h);
	struct symbol* sym = read_symbols(in);
	struct relocation* r_text = read_relocation(in, ".text");
	struct relocation* r_data = read_relocation(in, ".data");
	struct adjusted_relocation* ar_text = read_adjusted_relocations(in, ".text");
	struct adjusted_relocation* ar_data = read_adjusted_relocations(in, ".data");
	text_contents = read_segment(in, text->sh_offset, text->sh_size, ".text");
	data_contents = read_segment(in, data->sh_offset, data->sh_size, ".data");
	require(h->EI_CLASS == 1, "x86 is only 32bit\n");
	require(h->EI_DATA == 1, "x86 is only little endian\n");
	require(h->e_machine == 3, "elf file is not for x86\n");
	return EXIT_SUCCESS;
}

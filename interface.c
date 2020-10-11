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

struct buffer* get_file(FILE* f, char* name);
void architecture_load(struct buffer* in);
char* binary_name();
int page_size();
SCM realign_text_segments(struct elf_object_file* h);
void realign_data_segments(int page_size);
struct symbol* generate_symbol_table(struct elf_object_file* h);
struct relocation* collection_relocations(struct elf_object_file* f);
void apply_relocations();
void print_file(struct elf_object_file* f);
SCM Get_base_address();
int numerate_string(char *a);

int main(int argc, char** argv)
{
	struct buffer* in;
	struct elf_object_file* hold;
	FILE* destination_file;
	char* destination_name = "a.out";
	BaseAddress = Get_base_address();
	VERBOSE = FALSE;
	DEBUG = FALSE;
	text_size = 0;
	data_size = 0;
	int PrePRINT = FALSE;
	int PRINT = FALSE;

	int i = 1;
	while(i <= argc)
	{
		if(NULL == argv[i])
		{
			i = i + 1;
		}
		else if(match(argv[i], "-p") || match(argv[i], "--print"))
		{
			PRINT = TRUE;
			i = i + 1;
		}
		else if(match(argv[i], "-P") || match(argv[i], "--preprint"))
		{
			PrePRINT = TRUE;
			i = i + 1;
		}
		else if(match(argv[i], "--verbose"))
		{
			VERBOSE = TRUE;
			i = i + 1;
		}
		else if(match(argv[i], "-g") || match(argv[i], "--debug"))
		{
			DEBUG = TRUE;
			i = i + 1;
		}
		else if(match(argv[i], "-f") || match(argv[i], "--file"))
		{
			char* name = argv[i + 1];
			hold = current_file;
			current_file = calloc(1, sizeof(struct elf_object_file));
			current_file->next = hold;
			current_file->name = name;
			in = get_file(fopen(name, "r"), name);
			if(NULL == in)
			{
				file_print("Unable to open for reading file: ", stderr);
				file_print(name, stderr);
				file_print("\n Aborting to avoid problems\n", stderr);
				exit(EXIT_FAILURE);
			}

			architecture_load(in);
			require(current_file->header->e_type == 1, "M3-Meteoroid only supports linking relocatable files\n");
			i = i + 2;
		}
		else if(match(argv[i], "-o") || match(argv[i], "--output"))
		{
			destination_name = argv[i + 1];
			i = i + 2;
		}
		else if(match(argv[i], "-b") || match(argv[i], "--base-address"))
		{
			BaseAddress = numerate_string(argv[i + 1]);
			i = i + 2;
		}
		else if(match(argv[i], "-h") || match(argv[i], "--help"))
		{
			file_print("--file $input_file to set a file as input\n", stdout);
			file_print("--output $output_file to set the output file, otherwise output is to a.out\n", stdout);
			file_print("--debug for including sections\n", stdout);
			file_print("--verbose for more in depth error messages\n", stdout);
			file_print("--help for this message\n", stdout);
			file_print("--version for file version\n", stdout);
			exit(EXIT_SUCCESS);
		}
		else if(match(argv[i], "-V") || match(argv[i], "--version"))
		{
			file_print(binary_name(), stdout);
			file_print(" v0.0\n", stdout);
			exit(EXIT_SUCCESS);
		}
		else
		{
			file_print("UNKNOWN ARGUMENT\n", stdout);
			exit(EXIT_FAILURE);
		}
	}

	realign_text_segments(current_file);
	realign_data_segments(page_size());
	symbol_table = generate_symbol_table(current_file);
	relocation_table = collection_relocations(current_file);

	if(PrePRINT)
	{
		print_file(current_file);
		exit(EXIT_SUCCESS);
	}

	apply_relocations();

	if(PRINT)
	{
		print_file(current_file);
		exit(EXIT_SUCCESS);
	}

	destination_file = fopen(destination_name, "w");
	if(NULL == destination_file)
	{
		file_print("Unable to open for writing file: ", stderr);
		file_print(destination_name, stderr);
		file_print("\n Aborting to avoid problems\n", stderr);
		exit(EXIT_FAILURE);
	}


	return EXIT_SUCCESS;
}

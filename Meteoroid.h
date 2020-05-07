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

#include "gcc_req.h"

 // CONSTANT FALSE 0
#define FALSE 0
// CONSTANT TRUE 1
#define TRUE 1
// CONSTANT MAX_STRING 4096
#define MAX_STRING 4096

int match(char* a, char* b);
void file_print(char* s, FILE* f);
void require(int bool, char* error);

struct elf_header
{
	int EI_CLASS;
	int EI_DATA;
	int EI_VERSION;
	int EI_OSABI;
	int EI_ABIVERSION;
	int e_type;
	int e_machine;
	SCM e_entry;
	SCM e_phoff;
	SCM e_shoff;
	int e_flags;
	int e_ehsize;
	int e_phentsize;
	int e_phnum;
	int e_shentsize;
	int e_shnum;
	int e_shstrndx;
};

struct program_header
{
	int p_type;
	int p_flags;
	SCM p_offset;
	SCM p_vaddr;
	SCM p_paddr;
	SCM p_filesz;
	SCM p_memsz;
	SCM p_align;
	int program_header_number;
	struct program_header* next;
};

struct segment
{
	char* name;
	int size;
	char* contents;
	int starting_address;
};

struct section_header
{
	char* sh_name;
	int sh_name_offset;
	int sh_type;
	SCM sh_flags;
	SCM sh_addr;
	SCM sh_offset;
	SCM sh_size;
	int sh_link;
	int sh_info;
	SCM sh_addralign;
	SCM sh_entsize;
	int section_number;
	struct section_header* next;
};

struct symbol
{
	char* st_name;
	SCM st_name_offset;
	SCM st_value;
	SCM st_size;
	int st_info;
	int st_other;
	int st_shndx;
	int symbol_number;
	struct symbol* next;
};

struct relocation
{
	char* segment;
	SCM r_offset;
	SCM r_info;
	int relocation_number;
	struct relocation* next;
};

struct adjusted_relocation
{
	char* segment;
	SCM r_offset;
	int r_info;
	int r_info_top;
	SCM r_addend;
	int adjusted_relocation_number;
	struct adjusted_relocation* next;
};

/* Some globals to keep things simpler */
int BigEndian;
int largeint;
struct section_header* string_table;
struct section_header* symbol_table;
struct section_header* text;
struct section_header* data;
struct section_header* bss;
struct section_header* _rel_text;
struct section_header* _rela_text;
struct section_header* _rel_data;
struct section_header* _rela_data;
struct segment* text_contents;
struct segment* data_contents;

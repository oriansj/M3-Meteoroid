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

struct elf_program_header
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
	struct elf_program_header* next;
};

struct segment
{
	char* name;
	int size;
	char* contents;
	int starting_address;
};

struct elf_section_header
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
	struct segment* contents;
	struct elf_section_header* next;
};

struct elf_symbol
{
	char* st_name;
	SCM st_name_offset;
	SCM st_value;
	SCM st_size;
	int st_info;
	int st_other;
	int st_shndx;
	int symbol_number;
	struct elf_symbol* next;
};

struct symbol
{
	char* name;
	SCM address;
	struct symbol* next;
};

struct elf_relocation
{
	char* name;
	SCM r_offset;
	SCM r_info;
	SCM r_type;
	int relocation_number;
	struct elf_relocation* next;
};

struct elf_adjusted_relocation
{
	char* name;
	SCM r_offset;
	int r_info;
	int r_info_top;
	SCM r_addend;
	SCM r_type;
	int adjusted_relocation_number;
	struct elf_adjusted_relocation* next;
};

struct relocation
{
	char* symbol_name;
	struct elf_section_header* target_section;
	SCM target_offset;
	SCM type;
	struct relocation* next;
};

struct elf_object_file
{
	char* name;
	struct elf_header* header;
	struct elf_program_header* segments;
	struct elf_section_header* sections;
	struct elf_section_header* string_table;
	struct elf_section_header* symbol_table;
	struct elf_symbol* symbols;
	struct elf_section_header* text;
	struct elf_section_header* data;
	struct elf_section_header* bss;
	struct elf_section_header* _rel_text;
	struct elf_relocation* r_text;
	struct elf_section_header* _rela_text;
	struct elf_adjusted_relocation* ar_text;
	struct elf_section_header* _rel_data;
	struct elf_relocation* r_data;
	struct elf_section_header* _rela_data;
	struct elf_adjusted_relocation* ar_data;
	struct elf_object_file* next;
};

/* Some globals to keep things simpler */
int BigEndian;
int largeint;
SCM BaseAddress;
int VERBOSE;
int DEBUG;
struct elf_object_file* current_file;
struct symbol* symbol_table;
struct relocation* relocation_table;
struct symbol* entry;
SCM text_size;
SCM data_size;
SCM read_offset;
SCM write_offset;

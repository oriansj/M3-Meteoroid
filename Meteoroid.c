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

/* Some common functions */
int read_half(FILE* f, char* failure);
int read_word(FILE* f, char* failure);
int read_double(FILE* f, char* failure);
SCM read_register(FILE* f, char* failure);
char* read_string(FILE* f, int base, int offset, char* error);

struct elf_header* read_elf_header(FILE* f)
{
	struct elf_header* r = calloc(1, sizeof(struct elf_header));
	int c;
	int i;

	/* Read the Magic */
	fseek(f, 0, SEEK_SET);
	c = fgetc(f);
	require(EOF != c, "Hit EOF while attempting to read MAGIC0\n");
	require(0x7F == c, "First byte not 0x7F\n");
	c = fgetc(f);
	require(EOF != c, "Hit EOF while attempting to read MAGIC1\n");
	require('E' == c, "Second byte not E\n");
	c = fgetc(f);
	require(EOF != c, "Hit EOF while attempting to read MAGIC2\n");
	require('L' == c, "Third byte not L\n");
	c = fgetc(f);
	require(EOF != c, "Hit EOF while attempting to read MAGIC3\n");
	require('F' == c, "Fourth byte not F\n");

	/* Get if 32 or 64 bit */
	c = fgetc(f);
	require(EOF != c, "Hit EOF while attempting to read EI_CLASS\n");
	require((1 == c) || (2 == c), "Only 32 and 64bit supported\n");
	r->EI_CLASS = c;
	if(2 == c) largeint = TRUE;
	else largeint = FALSE;

	/* Figure out if big or little endian */
	c = fgetc(f);
	require(EOF != c, "Hit EOF while attempting to read EI_DATA\n");
	require((1 == c) || (2 == c), "Only big and little Endian supported\n");
	r->EI_DATA = c;
	if(2 == c) BigEndian = TRUE;
	else BigEndian = FALSE;

	/* Figure out which elf version */
	c = fgetc(f);
	require(EOF != c, "Hit EOF while attempting to read EI_VERSION\n");
	require(1 == c, "M3-Meteoroid only supports ELFv1 at this time\n");
	r->EI_VERSION = c;

	/* Figure which Operating system ABI expected (Usually just SysV) */
	c = fgetc(f);
	require(EOF != c, "Hit EOF while attempting to read EI_OSABI\n");
	r->EI_OSABI = c;

	/* Get kernel ABI but Linux kernel (after at least 2.6) has no definition of it;
	 * So glibc 2.12+ in case EI_OSABI == 3 treats this field as ABI version of the dynamic linker */
	c = fgetc(f);
	require(EOF != c, "Hit EOF while attempting to read EI_ABIVERSION\n");
	r->EI_ABIVERSION = c;

	/* Get the required NULL padding */
	for(i = 7; i > 0; i = i - 1)
	{
		c = fgetc(f);
		require(EOF != c, "Hit EOF while attempting to read EI_PAD\n");
		require(0 == c, "EI_PAD is expected to be NULLs\n");
	}

	/* Determine if this is a relocatable or executable file */
	r->e_type = read_half(f, "Hit EOF while attempting to read e_type\n");

	/* Figure out what architecture it is for */
	r->e_machine = read_half(f, "Hit EOF while attempting to read e_machine\n");

	/* Get e_version and make sure it is 1 */
	i = read_word(f, "Hit EOF while attempting to read e_version\n");
	require(1 == i, "M3-Meteoroid only supports ELF version 1 at this time\n");

	r->e_entry = read_register(f, "Hit EOF while attempting to read e_entry\n");
	r->e_phoff = read_register(f, "Hit EOF while attempting to read e_phoff\n");
	r->e_shoff = read_register(f, "Hit EOF while attempting to read e_shoff\n");
	r->e_flags = read_word(f, "Hit EOF while attempting to read e_flags\n");
	r->e_ehsize = read_half(f, "Hit EOF while attempting to read e_ehsize\n");
	r->e_phentsize = read_half(f, "Hit EOF while attempting to read e_phentsize\n");
	r->e_phnum = read_half(f, "Hit EOF while attempting to read e_phnum\n");
	r->e_shentsize = read_half(f, "Hit EOF while attempting to read e_shentsize\n");
	r->e_shnum = read_half(f, "Hit EOF while attempting to read e_shnum\n");
	r->e_shstrndx = read_half(f, "Hit EOF while attempting to read e_shstrndx\n");
	require(r->e_shstrndx <= r->e_shnum, "Index of the section header table entry exceeds number of section entries\n");

	return r;
}

struct program_header* read_program_header(FILE* f, struct elf_header* e)
{
	struct program_header* r = NULL;
	struct program_header* hold = NULL;
	int i;
	fseek(f, e->e_phoff, SEEK_SET);

	for(i = 0; i < e->e_phnum; i = i + 1)
	{
		r = calloc(1, sizeof(struct program_header));
		r->next = hold;
		r->p_type = read_word(f, "Hit EOF while attempting to read p_type\n");
		if(largeint) r->p_flags = read_word(f, "Hit EOF while attempting to read p_flags\n");
		r->p_offset = read_register(f, "Hit EOF while attempting to read p_offset\n");
		r->p_vaddr = read_register(f, "Hit EOF while attempting to read p_vaddr\n");
		r->p_paddr = read_register(f, "Hit EOF while attempting to read p_paddr\n");
		r->p_filesz = read_register(f, "Hit EOF while attempting to read p_filesz\n");
		r->p_memsz = read_register(f, "Hit EOF while attempting to read p_memsz\n");
		if(!largeint) r->p_flags = read_word(f, "Hit EOF while attempting to read p_flags\n");
		r->p_align = read_register(f, "Hit EOF wile attempting to read p_align\n");
		r->program_header_number = i;
		hold = r;
	}
	return r;
}

struct section_header* read_section_header(FILE* f, struct elf_header* e)
{
	struct section_header* r = NULL;
	struct section_header* hold = NULL;
	int i;
	SCM offset_of_strings = 0;
	fseek(f, e->e_shoff, SEEK_SET);
	for(i = 0; i < e->e_shnum; i = i + 1)
	{
		r = calloc(1, sizeof(struct section_header));
		r->next = hold;
		r->sh_name_offset = read_word(f, "Hit EOF while attempting to read sh_name offset\n");
		r->sh_type = read_word(f, "Hit EOF while attempting to read sh_type\n");
		r->sh_flags = read_register(f, "Hit EOF while attempting to read sh_flags\n");
		r->sh_addr = read_register(f, "Hit EOF while attempting to read sh_addr\n");
		r->sh_offset = read_register(f, "Hit EOF while attempting to read sh_offset\n");
		r->sh_size = read_register(f, "Hit EOF while attempting to read sh_size\n");
		r->sh_link = read_word(f, "Hit EOF while attempting to read sh_link\n");
		r->sh_info = read_word(f, "Hit EOF while attempting to read sh_info\n");
		r->sh_addralign = read_register(f, "Hit EOF while attempting to read sh_addralign\n");
		r->sh_entsize = read_register(f, "Hit EOF while attempting to read sh_entsize\n");
		r->section_number = i;
		hold = r;
		if(i == e->e_shstrndx) offset_of_strings = r->sh_offset;
	}

	while(NULL != hold)
	{
		hold->sh_name = read_string(f, offset_of_strings, hold->sh_name_offset, "Hit EOF while attempting to read sh_name string\n");
		if(match(".strtab", hold->sh_name)) string_table = hold;
		else if(match(".symtab", hold->sh_name)) symbol_table = hold;
		else if(match(".text", hold->sh_name)) text = hold;
		else if(match(".data", hold->sh_name)) data = hold;
		else if(match(".bss", hold->sh_name)) bss = hold;
		else if(match(".rel.text", hold->sh_name)) _rel_text = hold;
		else if(match(".rela.text", hold->sh_name)) _rela_text = hold;
		else if(match(".rel.data", hold->sh_name)) _rel_data = hold;
		else if(match(".rela.data", hold->sh_name)) _rela_data = hold;
		hold = hold->next;
	}

	return r;
}

struct symbol* read_symbols(FILE* f)
{
	struct symbol* r = NULL;
	struct symbol* hold = NULL;
	fseek(f, symbol_table->sh_offset, SEEK_SET);
	int i = 0;
	while(i < symbol_table->sh_info)
	{
		r = calloc(1, sizeof(struct symbol));
		r->next = hold;
		r->st_name_offset = read_word(f, "Hit EOF while attempting to read st_name offset\n");
		if(largeint)
		{
			r->st_info = fgetc(f);
			require(EOF != r->st_info, "Hit EOF while attempting to read st_info\n");
			r->st_other = fgetc(f);
			require(EOF != r->st_other, "Hit EOF while attempting to read st_other\n");
			r->st_shndx = read_half(f, "Hit EOF while attempting to read \n");
			r->st_value = read_register(f, "Hit EOF while attempting to read st_value\n");
			r->st_size = read_register(f, "Hit EOF while attempting to read st_size\n");
		}
		else
		{
			r->st_value = read_register(f, "Hit EOF while attempting to read st_value\n");
			r->st_size = read_register(f, "Hit EOF while attempting to read st_size\n");
			r->st_info = fgetc(f);
			require(EOF != r->st_info, "Hit EOF while attempting to read st_info\n");
			r->st_other = fgetc(f);
			require(EOF != r->st_other, "Hit EOF while attempting to read st_other\n");
			r->st_shndx = read_half(f, "Hit EOF while attempting to read \n");
		}
		r->symbol_number = i;
		hold = r;
		i = i + 1;
	}

	while(NULL != hold)
	{
		hold->st_name = read_string(f, string_table->sh_offset, hold->st_name_offset, "Hit EOF while attempting to read st_name\n");
		hold = hold->next;
	}

	return r;
}

struct relocation* read_relocation(FILE* f, char* segment)
{
	struct section_header* s = NULL;
	if(match(".data", segment)) s = _rel_data;
	else if(match(".text", segment)) s = _rel_text;
	else require(NULL != s, "read_relocation called withour valid section argument\n");

	if(NULL == s) return NULL;

	int count = s->sh_size / s->sh_entsize;
	struct relocation* r = NULL;
	struct relocation* hold = NULL;
	fseek(f, s->sh_offset, SEEK_SET);
	int i = 0;
	while(i < count)
	{
		r = calloc(1, sizeof(struct relocation));
		r->next = hold;
		r->segment = segment;
		r->r_offset = read_register(f, "Hit EOF while attempting to read r_offset\n");
		r->r_info = read_register(f, "Hit EOF while attempting to read r_info\n");

		r->relocation_number = i;
		hold = r;
		i = i + 1;
	}

	return r;
}

struct adjusted_relocation* read_adjusted_relocations(FILE* f, char* segment)
{
	struct section_header* s = NULL;
	if(match(".data", segment)) s = _rela_data;
	else if(match(".text", segment)) s = _rela_text;
	else require(NULL != s, "read_relocation called withour valid section argument\n");

	if(NULL == s) return NULL;

	int count = s->sh_size / s->sh_entsize;
	struct adjusted_relocation* r = NULL;
	struct adjusted_relocation* hold = NULL;
	fseek(f, s->sh_offset, SEEK_SET);
	int i = 0;
	while(i < count)
	{
		r = calloc(1, sizeof(struct adjusted_relocation));
		r->next = hold;
		r->segment = segment;
		r->r_offset = read_register(f, "Hit EOF while attempting to read r_offset\n");
		if(BigEndian && largeint) r->r_info_top = read_word(f, "Hit EOF while attempting to read r_info\n");
		r->r_info = read_word(f, "Hit EOF while attempting to read r_info\n");
		if(!BigEndian && largeint) r->r_info_top = read_word(f, "Hit EOF while attempting to read r_info\n");
		r->r_addend = read_register(f, "Hit EOF while attempting to read r_addend\n");

		r->adjusted_relocation_number = i;
		hold = r;
		i = i + 1;
	}

	return r;
}

struct segment* read_segment(FILE* f, int offset, int size, char* name)
{
	int i = 0;
	int c;
	struct segment* r = calloc(1, sizeof(struct segment));
	r->contents = calloc(size+1, sizeof(char));
	r->name = name;
	r->size = size;
	fseek(f, offset, SEEK_SET);
	while(i < size)
	{
		c = fgetc(f);
		require(EOF != c, "Hit EOF while attempting to read segment\n");
		r->contents[i] = c;
		i = i + 1;
	}

	return r;
}

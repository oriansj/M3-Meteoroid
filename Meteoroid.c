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
int read_half(struct segment* f, char* failure);
int read_word(struct segment* f, char* failure);
int read_double(struct segment* f, char* failure);
SCM read_register(struct segment* f, char* failure);
char* read_string(struct segment* f, int base, int offset, char* error);
int get_char(struct segment* f);
void put_char(int c, struct segment* f);
void write_half(struct segment* f, int o);
void write_word(struct segment* f, int o);
void write_double(struct segment* f, int o);
struct segment* output_buffer_generate();

struct elf_header* read_elf_header(struct segment* f)
{
	struct elf_header* r = calloc(1, sizeof(struct elf_header));
	int c;
	int i;

	/* Read the Magic */
	read_offset = 0;
	c = get_char(f);
	require(EOF != c, "Hit EOF while attempting to read MAGIC0\n");
	require(0x7F == c, "First byte not 0x7F\n");
	c = get_char(f);
	require(EOF != c, "Hit EOF while attempting to read MAGIC1\n");
	require('E' == c, "Second byte not E\n");
	c = get_char(f);
	require(EOF != c, "Hit EOF while attempting to read MAGIC2\n");
	require('L' == c, "Third byte not L\n");
	c = get_char(f);
	require(EOF != c, "Hit EOF while attempting to read MAGIC3\n");
	require('F' == c, "Fourth byte not F\n");

	/* Get if 32 or 64 bit */
	c = get_char(f);
	require(EOF != c, "Hit EOF while attempting to read EI_CLASS\n");
	require((1 == c) || (2 == c), "Only 32 and 64bit supported\n");
	r->EI_CLASS = c;
	if(2 == c) largeint = TRUE;
	else largeint = FALSE;

	/* Figure out if big or little endian */
	c = get_char(f);
	require(EOF != c, "Hit EOF while attempting to read EI_DATA\n");
	require((1 == c) || (2 == c), "Only big and little Endian supported\n");
	r->EI_DATA = c;
	if(2 == c) BigEndian = TRUE;
	else BigEndian = FALSE;

	/* Figure out which elf version */
	c = get_char(f);
	require(EOF != c, "Hit EOF while attempting to read EI_VERSION\n");
	require(1 == c, "M3-Meteoroid only supports ELFv1 at this time\n");
	r->EI_VERSION = c;

	/* Figure which Operating system ABI expected (Usually just SysV) */
	c = get_char(f);
	require(EOF != c, "Hit EOF while attempting to read EI_OSABI\n");
	r->EI_OSABI = c;

	/* Get kernel ABI but Linux kernel (after at least 2.6) has no definition of it;
	 * So glibc 2.12+ in case EI_OSABI == 3 treats this field as ABI version of the dynamic linker */
	c = get_char(f);
	require(EOF != c, "Hit EOF while attempting to read EI_ABIVERSION\n");
	r->EI_ABIVERSION = c;

	/* Get the required NULL padding */
	for(i = 7; i > 0; i = i - 1)
	{
		c = get_char(f);
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

struct elf_program_header* read_program_header(struct segment* f, struct elf_header* e)
{
	struct elf_program_header* r = NULL;
	struct elf_program_header* hold = NULL;
	int i;
	read_offset = e->e_phoff;

	for(i = 0; i < e->e_phnum; i = i + 1)
	{
		r = calloc(1, sizeof(struct elf_program_header));
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

struct elf_section_header* read_section_header(struct segment* f, struct elf_header* e)
{
	struct elf_section_header* r = NULL;
	struct elf_section_header* hold = NULL;
	int i;
	SCM offset_of_strings = 0;
	read_offset = e->e_shoff;
	for(i = 0; i < e->e_shnum; i = i + 1)
	{
		r = calloc(1, sizeof(struct elf_section_header));
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
		if(match(".strtab", hold->sh_name)) current_file->string_table = hold;
		else if(match(".symtab", hold->sh_name)) current_file->symbol_table = hold;
		else if(match(".text", hold->sh_name)) current_file->text = hold;
		else if(match(".data", hold->sh_name)) current_file->data = hold;
		else if(match(".bss", hold->sh_name)) current_file->bss = hold;
		else if(match(".rel.text", hold->sh_name)) current_file->_rel_text = hold;
		else if(match(".rela.text", hold->sh_name)) current_file->_rela_text = hold;
		else if(match(".rel.data", hold->sh_name)) current_file->_rel_data = hold;
		else if(match(".rela.data", hold->sh_name)) current_file->_rela_data = hold;
		hold = hold->next;
	}

	return r;
}

struct elf_symbol* read_symbols(struct segment* f)
{
	struct elf_symbol* r = NULL;
	struct elf_symbol* hold = NULL;
	read_offset = current_file->symbol_table->sh_offset;
	int i = 0;
	int count = current_file->symbol_table->sh_size / current_file->symbol_table->sh_entsize;
	if(current_file->symbol_table->sh_info != count)
	{
		file_print("\nWARNING: sh_info in the symbol table does not match number of entries\nPossible bug in assmbler/compiler that generated: ", stderr);
		file_print(current_file->name, stderr);
		file_print("\nPlease take note\n\n", stderr);
	}

	while(i < count)
	{
		r = calloc(1, sizeof(struct elf_symbol));
		r->next = hold;
		r->st_name_offset = read_word(f, "Hit EOF while attempting to read st_name offset\n");
		if(largeint)
		{
			r->st_info = get_char(f);
			require(EOF != r->st_info, "Hit EOF while attempting to read st_info\n");
			r->st_other = get_char(f);
			require(EOF != r->st_other, "Hit EOF while attempting to read st_other\n");
			r->st_shndx = read_half(f, "Hit EOF while attempting to read \n");
			r->st_value = read_register(f, "Hit EOF while attempting to read st_value\n");
			r->st_size = read_register(f, "Hit EOF while attempting to read st_size\n");
		}
		else
		{
			r->st_value = read_register(f, "Hit EOF while attempting to read st_value\n");
			r->st_size = read_register(f, "Hit EOF while attempting to read st_size\n");
			r->st_info = get_char(f);
			require(EOF != r->st_info, "Hit EOF while attempting to read st_info\n");
			r->st_other = get_char(f);
			require(EOF != r->st_other, "Hit EOF while attempting to read st_other\n");
			r->st_shndx = read_half(f, "Hit EOF while attempting to read \n");
		}
		r->symbol_number = i;
		hold = r;
		i = i + 1;
	}

	while(NULL != hold)
	{
		hold->st_name = read_string(f, current_file->string_table->sh_offset, hold->st_name_offset, "Hit EOF while attempting to read st_name\n");
		hold = hold->next;
	}

	return r;
}

char* find_relocation_symbol_name(SCM index)
{
	struct elf_symbol* i = current_file->symbols;
	while(NULL != i)
	{
		if(index == i->symbol_number)
		{
			/* first deal with happy case */
			if(!match("", i->st_name)) return i->st_name;

			/* deal with the case of a shit assembler */
			struct elf_section_header* sections = current_file->sections;
			while(NULL != sections)
			{
				if(i->st_shndx == sections->section_number) return sections->sh_name;

				sections = sections->next;
			}

			file_print("Giving up figuring out symbol\n", stderr);
			exit(EXIT_FAILURE);
		}

		i = i->next;
	}

	return NULL;
}

struct elf_relocation* read_relocation(struct segment* f, char* segment)
{
	struct elf_section_header* s = NULL;
	if(match(".data", segment))
	{
		s = current_file->_rel_data;
	}
	else if(match(".text", segment))
	{
		s = current_file->_rel_text;
	}
	else require(NULL != s, "read_relocation called withour valid section argument\n");

	if(NULL == s) return NULL;

	int count = s->sh_size / s->sh_entsize;
	struct elf_relocation* r = NULL;
	struct elf_relocation* hold = NULL;
	read_offset = s->sh_offset;
	int i = 0;
	while(i < count)
	{
		r = calloc(1, sizeof(struct elf_relocation));
		r->next = hold;
		r->r_offset = read_register(f, "Hit EOF while attempting to read r_offset\n");
		r->r_info = read_register(f, "Hit EOF while attempting to read r_info\n");
		r->name = find_relocation_symbol_name(r->r_info >> 8);
		r->r_type = r->r_info & 0xFF;

		r->relocation_number = i;
		hold = r;
		i = i + 1;
	}

	return r;
}

struct elf_adjusted_relocation* read_adjusted_relocations(struct segment* f, char* segment)
{
	struct elf_section_header* s = NULL;
	if(match(".data", segment))
	{
		s = current_file->_rela_data;
	}
	else if(match(".text", segment))
	{
		s = current_file->_rela_text;
	}
	else
	{
		file_print("read_relocation called withour valid section argument\n", stderr);
		exit(EXIT_FAILURE);
	}

	if(NULL == s) return NULL;

	int count = s->sh_size / s->sh_entsize;
	struct elf_adjusted_relocation* r = NULL;
	struct elf_adjusted_relocation* hold = NULL;
	read_offset = s->sh_offset;
	int i = 0;
	while(i < count)
	{
		r = calloc(1, sizeof(struct elf_adjusted_relocation));
		r->next = hold;
		r->r_offset = read_register(f, "Hit EOF while attempting to read r_offset\n");
		if(BigEndian && largeint) r->r_info_top = read_word(f, "Hit EOF while attempting to read r_info\n");
		r->r_info = read_word(f, "Hit EOF while attempting to read r_info\n");
		if(!BigEndian && largeint) r->r_info_top = read_word(f, "Hit EOF while attempting to read r_info\n");
		r->r_addend = read_register(f, "Hit EOF while attempting to read r_addend\n");
		r->name = find_relocation_symbol_name(r->r_info >> 8);
		r->r_type = r->r_info & 0xFF;

		r->adjusted_relocation_number = i;
		hold = r;
		i = i + 1;
	}

	return r;
}

struct segment* read_segment(struct segment* f, int offset, int size, char* name)
{
	int i = 0;
	int c;
	struct segment* r = calloc(1, sizeof(struct segment));
	r->starting_address = -1;
	r->contents = calloc(size+4, sizeof(char));
	r->name = name;
	r->size = size;
	read_offset = offset;
	while(i < size)
	{
		c = get_char(f);
		require(EOF != c, "Hit EOF while attempting to read segment\n");
		r->contents[i] = c;
		i = i + 1;
	}

	return r;
}


SCM calculate_next_start(struct segment* s, int page_size)
{
	/* require sizeof(SCM) of padding after segments because I feel like it */
	SCM last_address = s->starting_address + s->size + sizeof(SCM);
	if(0 == (last_address & (page_size - 1))) return last_address;
	return (last_address & ~(page_size - 1)) + page_size;
}


void read_elf_file(struct segment* in)
{
	current_file->header = read_elf_header(in);
	current_file->segments = read_program_header(in, current_file->header);
	current_file->sections = read_section_header(in, current_file->header);
	current_file->symbols = read_symbols(in);
	current_file->r_text = read_relocation(in, ".text");
	current_file->r_data = read_relocation(in, ".data");
	current_file->ar_text = read_adjusted_relocations(in, ".text");
	current_file->ar_data = read_adjusted_relocations(in, ".data");

	/* Don't try t read .TEXT if it is not there */
	if(NULL != current_file->text)
	{
		current_file->text->contents = read_segment(in, current_file->text->sh_offset, current_file->text->sh_size, ".text");
		text_size = text_size + current_file->text->sh_size;
	}

	/* Don't try t read .DATA if it is not there */
	if(NULL != current_file->data)
	{
		current_file->data->contents = read_segment(in, current_file->data->sh_offset, current_file->data->sh_size, ".data");
		data_size = data_size + current_file->data->sh_size;
	}
}

struct elf_object_file* reverse_nodes(struct elf_object_file* head)
{
	struct elf_object_file* root = NULL;
	while(NULL != head)
	{
		struct elf_object_file* next = head->next;
		head->next = root;
		root = head;
		head = next;
	}
	return root;
}

SCM realign_text_segments(struct elf_object_file* h)
{
	if(NULL == h) return BaseAddress;
	if(NULL == h->text) return realign_text_segments(h->next);

	h->text->contents->starting_address = realign_text_segments(h->next);
	return h->text->contents->starting_address + h->text->contents->size;
}

void realign_data_segments(int page_size)
{
	SCM data_start = calculate_next_start(current_file->text->contents, page_size);
	current_file = reverse_nodes(current_file);
	struct elf_object_file* h = current_file;
	while(NULL != h)
	{
		if(NULL != h->data)
		{
			h->data->contents->starting_address = data_start;
			data_start = data_start + h->data->contents->size;
		}
		h = h->next;
	}
}

SCM lookup_section(char* s, struct elf_section_header* t)
{
	if(NULL == t) return -1;

	if(match(s, t->sh_name)) return t->section_number;

	return lookup_section(s, t->next);
}

void check_for_duplicate_symbols(char* s, struct symbol* sym)
{
	while(NULL != sym)
	{
		if(match(s, sym->name))
		{
			file_print("duplicate definition found for: ", stderr);
			file_print(s, stderr);
			file_print("\nAborting to prevent issues\n", stderr);
			exit(EXIT_FAILURE);
		}

		sym = sym->next;
	}
}

struct symbol* generate_symbol_table(struct elf_object_file* h)
{
	if(NULL == h) return NULL;

	SCM text_index = lookup_section(".text", h->symbol_table);
	SCM data_index = lookup_section(".data", h->symbol_table);

	struct symbol* r = generate_symbol_table(h->next);
	struct symbol* hold = NULL;
	struct elf_symbol* i;
	for(i = h->symbols; NULL != i; i = i->next)
	{
		/* Only add if have name and is not undefined */
		if(!match("", i->st_name) && (0 != i->st_shndx))
		{
			check_for_duplicate_symbols(i->st_name, r);
			hold = r;
			r = calloc(1, sizeof(struct symbol));
			r->name = i->st_name;

			if(text_index == i->st_shndx)
			{
				r->address = h->text->contents->starting_address + i->st_value;
			}
			else if(data_index == i->st_shndx)
			{
				r->address = h->data->contents->starting_address + i->st_value;
			}
			else if(0xFFF1 == i->st_shndx)
			{
				/* It is an absolute address */
				r->address = i->st_value;
			}
			else
			{
				file_print("I just got an st_shndx value I don't understand\nAborting so I don't miss something\n", stderr);
				exit(EXIT_FAILURE);
			}

			r->next = hold;
		}
	}

	return r;
}

char* find_address_symbol_name(SCM address)
{
	if(0 > address) return NULL;
	struct symbol* s = symbol_table;
	while(NULL != s)
	{
		if(address == s->address) return s->name;
		s = s->next;
	}

	return NULL;
}

SCM get_address_from_symbol(char* name)
{
	require(NULL != name, "It is not possible to get the address when you don't give me a symbol's name\n");

	struct symbol* s = symbol_table;
	while(NULL != s)
	{
		if(match(name, s->name)) return s->address;
		s = s->next;
	}

	file_print("Was unable to find symbol named: ", stderr);
	file_print(name, stderr);
	file_print(" in the symbol table\nAborting before I do something stupid\n", stderr);
	exit(EXIT_FAILURE);
}

struct relocation* collection_relocations(struct elf_object_file* f)
{
	if(NULL == f) return NULL;

	struct relocation* r = collection_relocations(f->next);
	struct relocation* hold = NULL;
	struct elf_relocation* a = f->r_text;
	SCM offset = -1;

	while(NULL != a)
	{
		/* Because ELF shoves the offset into where the value belongs to save disk space; we need to pull it out */
		read_offset = a->r_offset;
		offset = read_word(f->text->contents, "failed to read .data relocation offset from segment\n");

		/* Create our useful relocation record (pointing to the old) */
		hold = r;
		r = calloc(1, sizeof(struct relocation));
		r->next = hold;
		r->target_section = f->text;
		r->target_offset = a->r_offset;

		/* Depending if the relocation actually gave us the name or the segment where to find it */
		r->symbol_name = a->name;
		if(match(".data", a->name)) r->symbol_name = find_address_symbol_name(f->data->contents->starting_address + offset);
		if(match(".text", a->name)) r->symbol_name = find_address_symbol_name(f->text->contents->starting_address + offset);

		a = a->next;
	}

	return r;
}


void write_elf_header(struct segment* f)
{
	read_offset = 0;
	/* put in the magic */
	put_char('\x7f', f);
	put_char('E', f);
	put_char('L', f);
	put_char('F', f);
	put_char(current_file->header->EI_CLASS, f);
	put_char(current_file->header->EI_DATA, f);
	put_char(current_file->header->EI_VERSION, f);
	put_char(current_file->header->EI_OSABI, f);
	put_char(current_file->header->EI_ABIVERSION, f);
	/* EI_PAD */
	put_char(0, f);
	put_char(0, f);
	put_char(0, f);
	put_char(0, f);
	put_char(0, f);
	put_char(0, f);
	put_char(0, f);
	/* set e_type to ET_EXEC */
	write_half(f, 2);
	write_half(f, current_file->header->e_machine);
	/* Set e_version to 1 */
	write_word(f, 1);
}

struct segment* output_generate()
{
	struct segment* r = output_buffer_generate();
	write_elf_header(r);
	return r;
}

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

void read_elf_file(struct buffer* in);
void architecture_load(struct buffer* in)
{
	read_elf_file(in);
	require(current_file->header->e_machine == 3, "elf file is not for x86\n");
	require(current_file->header->EI_CLASS == 1, "x86 is only 32bit\n");
	require(current_file->header->EI_DATA == 1, "x86 is only little endian\n");
}

char* binary_name()
{
	return "M3-Meteoroid-x86";
}

int page_size()
{
	/* Assume page size is 4KB */
	return 4096;
}

SCM Get_base_address()
{
	return 0x8048000;
}

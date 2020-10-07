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

// CONSTANT FALSE 0
#define FALSE 0
// CONSTANT TRUE 1
#define TRUE 1

int char2hex(int c)
{
	if (c >= '0' && c <= '9') return (c - 48);
	else if (c >= 'a' && c <= 'f') return (c - 87);
	else if (c >= 'A' && c <= 'F') return (c - 55);
	else return -1;
}

int char2dec(int c)
{
	if (c >= '0' && c <= '9') return (c - 48);
	else return -1;
}

int numerate_string(char *a)
{
	int count = 0;
	int index;
	int negative;

	/* If NULL string */
	if(0 == a[0])
	{
		return 0;
	}
	/* Deal with hex */
	else if (a[0] == '0' && a[1] == 'x')
	{
		if('-' == a[2])
		{
			negative = TRUE;
			index = 3;
		}
		else
		{
			negative = FALSE;
			index = 2;
		}

		while(0 != a[index])
		{
			if(-1 == char2hex(a[index])) return 0;
			count = (16 * count) + char2hex(a[index]);
			index = index + 1;
		}
	}
	/* Deal with decimal */
	else
	{
		if('-' == a[0])
		{
			negative = TRUE;
			index = 1;
		}
		else
		{
			negative = FALSE;
			index = 0;
		}

		while(0 != a[index])
		{
			if(-1 == char2dec(a[index])) return 0;
			count = (10 * count) + char2dec(a[index]);
			index = index + 1;
		}
	}

	if(negative)
	{
		count = count * -1;
	}
	return count;
}

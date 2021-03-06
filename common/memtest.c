/*
 * common/memtest.c
 *
 * Copyright (C) 2010 Barnes & Noble, Inc.
 *
 * RAM test functions
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/



#include "memtest.h"


/* http://www.ganssle.com/testingram.htm   - Inverting Bits section
     use a 257-byte long pattern to perform a basic march test.

    From Ken Tough:
    The long pattern string should contain a section like this as well as some more random bytes:
    0xFFFF 0xFFFF 0xFFFF 0xFFFF
    0x0000 0x0000 0x0000 0x0000
    0xFFFF 0xFFFF 0xFFFF 0xFFFF
    0x0000 0xAAAA 0xAAAA 0x0000
    0x5555 0x5555 0x5555 0x5555
    0xAAAA 0xAAAA 0xAAAA 0xAAAA

    That kind of pattern will make sure we have a part with full bit transitions to capture things l
    ike groundbounce caused by simultaneous output switching.      

*/
static const u8 long_pattern[] = 
{
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0xAA, 0xAA, 0xAA, 0xAA, 0x00, 0x00, 
	0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 
	0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00
}; 

/*
    Define a struct to facilitate 257-byte block memory write, 
    instead of writing memory byte by byte.
*/

struct LONG_PATTERN
{
    u8 long_pattern[sizeof(long_pattern)]; 
};


void memtest_Mats_blk(u8* mem_start, u8* mem_end)
{
    volatile u8* dst         = mem_start;
             u8* src;
    volatile u8* pattern     = long_pattern;
             u8* pattern_end = long_pattern + sizeof(long_pattern) - 1;
    
    int num_pattern = (mem_end - mem_start + 1) / sizeof(long_pattern);
    int loop;
   
    printf ("Filling 0x%08x ... 0x%08x\n", (uint)mem_start, (uint)mem_end);

    /* fill up the test area block by block (257 byte) */
    for(loop = 0; loop < num_pattern; loop++)
    {
        *((struct LONG_PATTERN*)dst) = *((struct LONG_PATTERN*)long_pattern);
        dst += sizeof(long_pattern);
    }

    /* fill up the remaining memory area */
    for(src = long_pattern; dst <= mem_end; dst++, src++)
    {
        *dst = *src;
    }

    printf("Validating......\n");

    /* validate memory value byte by byte */
    dst = mem_start;
    pattern     = long_pattern;    
    while(dst <= mem_end)
    {
        if(*dst != *pattern)
        {
            printf("Error2@0x%x, value=0x%x, but shall be 0x%x.\n", (uint)dst, (uint)*dst, (uint)*pattern);
            return;
        };
        dst++;
        pattern++;

        if(pattern > pattern_end)
        {
            pattern = long_pattern;
        }
    }

    printf("Memory test passed.\n");
    
    return;    
}




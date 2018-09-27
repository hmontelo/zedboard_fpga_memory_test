/*
 * memory_api.c
 * author: Team 3
 * Created: Feb 14, 2018
 *
 *  dm.c, pm.c
 *
 *  Simple utility to allow the use of the /dev/mem device to display memory
 *  and write memory addresses on the Xilinx Zedboard.
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
 *
 */

#include "stdio.h"
#include "stdlib.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "memory_api.h"

#define MAP_SIZE 4096UL
#define MAP_MASK (MAP_SIZE - 1)

int dm(unsigned long addr, unsigned int* dat) {

/*
* This section of code is needed if you are accessing FLASH memory. the mmap() routine
* seems to leave the flash in a strange state after the first access
*/

	int fd = open("/dev/mem", O_RDWR|O_SYNC, S_IRUSR);
	volatile unsigned int *regs, *address ;

	unsigned long target_addr, offset, value;

	if(fd == -1)
	{
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}

	offset = 0;
	target_addr =  addr; //strtoul(argv[1], 0, 0);

    regs = (unsigned int *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, target_addr & ~MAP_MASK);


	address = regs + (((target_addr+ offset) & MAP_MASK)>>2);
  
  // put result into data pointer
	*dat = *address; // display register value
	offset  += 4;    // WORD alligned

	int temp = close(fd);
	if(temp == -1)
	{
		printf("Unable to close /dev/ram1.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}

	munmap(NULL, MAP_SIZE);

	return 0;
}

int pm(unsigned long addr, unsigned int* dat) {

	int fd = open("/dev/mem", O_RDWR|O_SYNC);
	volatile unsigned int *regs, *address ;

	unsigned int target_addr, offset, value;

	if(fd == -1)
	{
		printf("Unable to open /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}

	offset = 0;
	target_addr = addr; //strtoul(argv[1], 0, 0);

	regs = (unsigned int *)mmap(NULL, MAP_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, fd, target_addr & ~MAP_MASK);


  address = regs + (((target_addr+ offset) & MAP_MASK)>>2);
	value = *dat; //strtoul(argv[2], 0, 0);
	*address = value; 						// perform write command

	int temp = close(fd);
	if(temp == -1)
	{
		printf("Unable to close /dev/mem.  Ensure it exists (major=1, minor=1)\n");
		return -1;
	}

	munmap(NULL, MAP_SIZE);

	return 0;
}

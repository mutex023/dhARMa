/*
* signGP - https://beagleboard.googlecode.com/files/signGP.c
* Modified for BeagleBoneBlack by,
* @author: muteX023
*/

//
// signGP.c
// Read the x-load.bin file and write out the x-load.bin.ift file.
// The signed image is the original pre-pended with the size of the image
// and the load address.  If not entered on command line, file name is
// assumed to be x-load.bin in current directory and load address is
// 0x40200800.

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <malloc.h>

unsigned short tocCfgHeader[48] = {
							0x0040, 0x0000, 0x000c, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
							0x0000, 0x0000, 0x4843, 0x4553, 0x5454, 0x4e49, 0x5347, 0x0000,
							0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
							0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,							
							0xc0c1, 0xc0c0, 0x0100, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
							0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000
							};


main(int argc, char *argv[])
{
	int	i, j;
	char	ifname[FILENAME_MAX], ofname[FILENAME_MAX], ch;
	FILE	*ifile, *ofile;
	unsigned long	loadaddr, len;
	struct stat	sinfo;
	unsigned char zero = 0;

	// Default to x-load.bin and 0x40200800.
	strcpy(ifname, "x-load.bin");
	//loadaddr = 0x40200800;
	loadaddr = 0x402f0400; // BBB internal SRAM start addr

	if ((argc == 2) || (argc == 3))
		strcpy(ifname, argv[1]);

	if (argc == 3)
		loadaddr = strtol(argv[2], NULL, 16);

	// Form the output file name.
	strcpy(ofname, ifname);
	strcat(ofname, ".ift");

	// Open the input file.
	ifile = fopen(ifname, "rb");
	if (ifile == NULL) {
		printf("Cannot open %s\n", ifname);
		exit(0);
	}

	// Get file length.
	stat(ifname, &sinfo);
	len = sinfo.st_size;

	// Open the output file and write it.
	ofile = fopen(ofname, "wb");
	if (ofile == NULL) {
		printf("Cannot open %s\n", ofname);
		fclose(ifile);
		exit(0);
	}

	// Pad 1 sector of zeroes.
	//ch = 0x00;
	//for (i=0; i<0x200; i++)
	//	fwrite(&ch, 1, 1, ofile);
	
	//add the TOC configuration header (see ARM335x TRM sec 26.1.7.5.5)
	fwrite(tocCfgHeader, 1, 96, ofile);
	for(i = 0; i < 416; i++)
		fwrite(&zero, 1, 1, ofile);

	fwrite(&len, 1, 4, ofile);
	fwrite(&loadaddr, 1, 4, ofile);
	for (i=0; i<len; i++) {
		fread(&ch, 1, 1, ifile);
		fwrite(&ch, 1, 1, ofile);
	}

	fclose(ifile);
	fclose(ofile);
}

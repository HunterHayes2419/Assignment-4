#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * Returns FILE pointer from filename and debug flag
 * @param filename	the filename of the file to read
 * @param debug		prints debug statements to console
 * @return			the file pointer to the filename
 */

FILE* getFilePointer(char* filename, char* flags, bool debug){
	FILE *fp;
	if (debug)
		printf("Attempting to open %s...\n", filename);

	fp = fopen(filename, flags);

	if (fp == NULL){
		if(debug)
			printf("Something went wrong! Aborting...\n");
		exit(255);
	}
	else if (debug) {
		printf("Opened %s!\n\n", filename);
	}

	return fp;
}
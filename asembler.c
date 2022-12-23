#include <stdio.h>
#include "asemblerUtils.h"


/*manages the sending of files for execution*/
int main(int argc, char* argv[])
{
	/* check for amount of files */
	if (argc < 2) {
		printf("Eror: There were no file found. \n");
		exit(0);
	}

	/*processing each file*/
	while (--argc) {
		process_file(argv[argc]);
	}

	return 0;
}
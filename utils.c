#include "utils.h"


bool getNextWord(char* word, char** chptr, int size)
{
	bool retVal = false;

	/*initializing word*/
	memset(word, '\0', size);

	/*runing to end of white spaces*/
	while (**chptr == ' ' || **chptr == '\t')
		(*chptr)++;
	if (**chptr != '\n' && **chptr != EOF)/*if the row dident end*/
	{
		retVal = true;
		*word = **chptr;
		word++;
		(*chptr)++;
		/*inserting everithing until white space*/
		while (**chptr != ' ' && **chptr != '\t' && **chptr != '\n' && **chptr != EOF)
		{
			*word = **chptr;
			word++;
			(*chptr)++;
		}
	}
	return retVal;
}

void get_base_addres_and_offset(int address, int* base, int* offset)
{
	*offset = address % 16;
	*base = address - *offset;
}


void printWord(FILE* objectFile, int IC, codeLine* ptrMemory)
{
	int A, B, C, D, E;

	A = ptrMemory->A * 4;
	A += ptrMemory->R * 2;
	A += ptrMemory->E;
	B = (ptrMemory->dataWord.opcodeOrData & (0xF000)) >> 12;
	C = (ptrMemory->dataWord.opcodeOrData & (0x0F00)) >> 8;
	D = (ptrMemory->dataWord.opcodeOrData & (0x00F0)) >> 4;
	E = ptrMemory->dataWord.opcodeOrData & (0x000F);

	fprintf(objectFile, "%04d  A%x-B%x-C%x-D%x-E%x\n", IC, A, B, C, D, E);
}

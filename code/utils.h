#pragma once
#ifndef _UTILS_STRING 
#define _UTILS_STRING

#include <stdio.h>
#include "asemblerUtils.h"

/*get the next word from chptr in "word"*/
bool getNextWord(char* word, char** chptr, int size);


/*fills base and offset of an addres in the given places*/
void get_base_addres_and_offset(int address, int* base, int* offset);

/*inserts row to the .ob file by format*/
void printWord(FILE* objectFile, int IC, codeLine* ptrMemory);


#endif /*_UTILS_STRING*/

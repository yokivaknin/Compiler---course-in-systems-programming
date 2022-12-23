#include "macro.h"

/*if the word given is a register then returns its number else returns -1*/
int get_register_number(char* word)
{
	int result = -1;
	int i;
	
	/*for all registers*/
	for (i = 0; i < 16; i++)
	{
		char reg[4] = "r";
		sprintf(reg, "%s%d", reg, i);
		if (strcmp(word, reg) == 0)
			result = i;
	}

	return result;
}

/*returns true if the name given is legal else returns false*/
bool is_valid_name(char* word)
{
	bool result = true;
	int i;
	/*if is a command name*/
	for (i = 0; i < 16; i++)
	{
		if (strcmp(opcode_data[i].command_name, word) == 0)
			result = false;
	}

	if (result != false)
	{
		/*if is a directive or start with number or a register*/
		if (strcmp(word, DATA) == 0 || 
			strcmp(word, ENTRY) == 0 || 
			strcmp(word, EXTERN) == 0 || 
			strcmp(word, STRING) == 0 ||
			strcmp(word, "macro") == 0 ||
			strcmp(word, "endm") == 0 ||
			(word[0] >= '0' && word[0] <= '9') ||
			get_register_number(word) != -1)

			result = false;
	}

	return result;
}

/*finds macro in the macro_list and returns a pointer to it. if macro dont exist return null*/
macro_list* get_macro(char* macroName, macro_list* macroTableHead)
{
	macro_list* retVal = NULL;

	macro_list* ptrMacro = macroTableHead;
	
	/*runing on the macro list*/
	while (ptrMacro != NULL)
	{
		/*checking for the correct name*/
		if (strcmp(ptrMacro->name, macroName) == 0)
		{
			retVal = ptrMacro;
			break;
		}
		/*advancing the pointer*/
		ptrMacro = ptrMacro->next;
	}
	
	return retVal;
}

/*in case of success returning pointer to new file with macros opend, in failure returning null*/
FILE* process_macro(FILE* current, char* cleanFileName)
{
	FILE* product;
	char* macroFileName;
	int rowNumber = 0;

	macro_list* macroTable;
	macro_list* ptrcleaner;
	macro_list* ptrToMacro;

	/*row from the .as file given*/
	char row[MAX_ROW_LENGTH];

	/*a word from the row*/
	char word[MAX_ROW_LENGTH];
	char macroName[MAX_ROW_LENGTH];
	bool isMacroValid = true;

	/*creating the .am file name*/
	macroFileName = malloc(strlen(cleanFileName)+4);
	strcpy(macroFileName, cleanFileName);
	strcat(macroFileName, ".am");
	
	/*creating the .am file*/
	product = fopen(macroFileName, "w+");
	macroTable = NULL;
	
	/*runing for eatch row in the .as file*/
	while (fgets(row, MAX_ROW_LENGTH, current))
	{
		rowNumber++;
		/*geting the first word in the row*/
		sscanf(row,"%s", word);

		/*checking if the word is in the macro list*/
		ptrToMacro = get_macro(word, macroTable);
		if (ptrToMacro != NULL)
		{
			macro_content* ptrContent = ptrToMacro->content;
			/*inserting the macro in the new file*/
			while (ptrContent->next != NULL)
			{
				fputs(ptrContent->macro_row_content, product);
				ptrContent = ptrContent->next;
			}
			fputs(ptrContent->macro_row_content, product);
			continue;
		}

		/*if the word is not macro*/
		if (strcmp(word, "macro") != 0)
			fputs(row, product);/*inserting the row to the new file*/
		else
		{
			macro_content* currentPtr = NULL;
			macro_content* prevPtr = NULL;

			/*geting the macro name*/
			sscanf(row,"%s %s", word, macroName);
			/*checking the name is legal*/
			if (is_valid_name(macroName))
			{
				/*allocating memory for the block*/
				macro_list* macroTableBlock =  malloc(sizeof(macro_list));
				if (macroTableBlock != NULL)
				{
					strcpy(macroTableBlock->name, macroName);
					macroTableBlock->content = NULL;

					macroTableBlock->next = NULL;

					fgets(row, MAX_ROW_LENGTH, current);
					rowNumber++;
					sscanf(row, "%s", word);
					/*inserting the new macro to the macro list*/
					while (strcmp(word, "endm") != 0)
					{
						currentPtr = malloc(sizeof(macro_content));
						if (currentPtr != NULL)
						{
							strcpy(currentPtr->macro_row_content, row);
							currentPtr->next = NULL;
							
							if (macroTableBlock->content == NULL)
							{
								prevPtr = currentPtr;
								macroTableBlock->content = currentPtr;
							}
							else
							{
								prevPtr->next = currentPtr;
								prevPtr = currentPtr;
							}
						}

						fgets(row, MAX_ROW_LENGTH, current);
						rowNumber++;
						sscanf(row, "%s", word);
					}
				}
				else
				{
					printf("System eror: there was an memory alloction problem at macro procesing for file: %s.\n", macroFileName);
					isMacroValid = false;
				}
				if (macroTable == NULL)
				{
					macroTable = macroTableBlock;
				 }
				else
				{
					macroTableBlock->next = macroTable;
					macroTable = macroTableBlock;
				}	
			}
			else/*macro name is not valid*/
			{
				isMacroValid = false;
				printf("Eror in file: %s row: %d macro name is not valid", macroFileName, rowNumber);
			}	
		}
	}

	/*free all allocated space*/
	ptrcleaner = macroTable;
	while (macroTable != NULL)
	{
		macro_content* head;
		macro_content* cleaner;
		ptrcleaner = ptrcleaner->next;

		head = macroTable->content;
		cleaner = macroTable->content;
		while (head != NULL)
		{
			cleaner = cleaner->next;
			free(head);
			head = cleaner;
		}

		free(macroTable);
		macroTable = ptrcleaner;
	}

	/*returning result*/
	if (isMacroValid == true)
	{
		free(macroFileName);
		return product;
	}
	else
	{
		fclose(product);
		remove(macroFileName);

		free(macroFileName);
		return NULL;
	}
}

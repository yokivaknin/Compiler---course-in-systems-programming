#include <ctype.h>
#include <math.h>
#include "asemblerUtils.h"
#include "macro.h"

/*start of globals*/

int IC = BASE_IC_ADDRESS;/*instruction counter*/
int DC = 0;/*data counter*/

int ICF = 0;
int DCF = 0;

/*data table head*/
codeLine* dataTable = NULL;
codeLine* pointerToEndOfDataTable;

/*instruction table head*/
codeLine* instructionTable = NULL;
codeLine* pointerToEndOfInstructionTable;

/*symbol table head*/
symbol* symbolTable = NULL;

/*instruction table pointer for the use of the second loop*/
codeLine* ptrIC;

/*for eror reporting*/
char* fullFileName;

/*end of globals*/

/*opcode table initialization*/
opcode_table opcode_data[16] =
{
	{"mov",1,-1,2,{true,true,true,true},{false,true,true,true}},
	{"cmp",2,-1,2,{true,true,true,true},{true,true,true,true}},
	{"add",4,10,2,{true,true,true,true},{false,true,true,true}},
	{"sub",4,11,2,{true,true,true,true},{false,true,true,true}},
	{"lea",16,-1,2,{false,true,true,false},{false,true,true,true}},
	{"clr",32,10,1,{false,false,false,false},{false,true,true,true}},
	{"not",32,11,1,{false,false,false,false},{false,true,true,true}},
	{"inc",32,12,1,{false,false,false,false},{false,true,true,true}},
	{"dec",32,13,1,{false,false,false,false},{false,true,true,true}},
	{"jmp",512,10,1,{false,false,false,false},{false,true,true,false}},
	{"bne",512,11,1,{false,false,false,false},{false,true,true,false}},
	{"jsr",512,12,1,{false,false,false,false},{false,true,true,false}},
	{"red",4096,-1,1,{false,false,false,false},{false,true,true,true}},
	{"prn",8192,-1,1,{false,false,false,false},{true,true,true,true}},
	{"rts",16384,-1,0,{false,false,false,false},{false,false,false,false}},
	{"stop",32768,-1,0,{false,false,false,false},{false,false,false,false}}
};


void process_file(char* fileName)
{
	FILE* file;

	/*creating the full file name*/
	fullFileName = malloc(strlen(fileName) + 4);
	strcpy(fullFileName, fileName);
	fullFileName = strcat(fullFileName, ".as");

	/*opening the file*/
	if ((file = fopen(fullFileName, "r")) == NULL)
	{
		printf("The file: %s can not be open.\n", fullFileName);
		return;
	}
	else
	{
		/*geting .am file after macro process*/
		FILE* opendMacroFile = process_macro(file, fileName);
		if (opendMacroFile == NULL)
			return;
		if (first_loop(opendMacroFile) == false)
		{
			reset_all(fullFileName);
			return;
		}
		if (second_loop(opendMacroFile, fileName) == false)
		{
			reset_all(fullFileName);
			return;
		}
		create_product(fileName);
		reset_all(fullFileName);
	}
}


symbol* is_symbol_exist(char* to_comper)
{
	symbol* head = symbolTable;
	if (head == NULL)
		return NULL;
	/*runing on all the symbol table*/
	while (head != NULL)
	{
		if (strcmp(head->name, to_comper) == 0)
			return head;
		head = head->next;
	}

	return head;
}


bool is_label(char* lable)
{
	bool result = true;
	int length;
	char currentCharacter;

	length = strlen(lable);

	/*legal label is max 31 characters */
	if (length > 31)
		result = false;
	else
	{
		length = 0;
		currentCharacter = lable[length];
		/*if the first char isen't a letter*/
		if (isalpha(currentCharacter) == false)
		{
			result = false;
		}
		else
		{
			length++;
			/*for each char if he is not a letter or a nunber*/
			while (lable[length] != '\0')
			{
				currentCharacter = lable[length];
				if (isalnum(currentCharacter) == false)
				{
					result = false;
					break;
				}
				length++;
			}
		}
	}
	/*if lable is not a saved worde*/
	if (result == true)
	{
		result = is_valid_name(lable);
	}
	return result;
}


bool insert_data_to_mem(char* row, char* dataType, int lineNumber)
{
	codeLine* block;

	if (strcmp(dataType, STRING) == 0)
	{
		char data[MAX_ROW_LENGTH];
		int i;
		/*advencing row to the character after .string*/
		row = strstr(row, STRING) + strlen(STRING);
		if (sscanf(row, "%s", data) != EOF)
		{
			/*checking for apostrophes*/
			if ((data[0] == '"') || (data[strlen(data) - 1] == '"'))
			{
				for (i = 1; i < strlen(data); i++)
				{
					if (i < strlen(data) - 1)/*the actual data*/
					{
						block = malloc(sizeof(codeLine));
						if (block == NULL)
						{
							printf("System eror in file: %s row: %d there was an memory alloction problem.\n", fullFileName, lineNumber);
							return false;
						}
						/*block creation*/
						block->A = 1;
						block->R = 0;
						block->E = 0;
						block->last_bit = 0;
						block->next = NULL;
						block->dataWord.opcodeOrData = data[i];
						DC++;
					}
					else/*the last data word '\0' */
					{
						block = malloc(sizeof(codeLine));
						if (block == NULL)
						{
							printf("System eror in file: %s row: %d there was an memory alloction problem.\n", fullFileName, lineNumber);
							return false;
						}
						/*block creation*/
						block->A = 1;
						block->R = 0;
						block->E = 0;
						block->last_bit = 0;
						block->next = NULL;
						block->dataWord.opcodeOrData = 0;
						DC++;
					}


					/*inserting the block to the data table*/
					if (dataTable == NULL)
					{
						dataTable = block;
						pointerToEndOfDataTable = block;
					}
					else
					{
						pointerToEndOfDataTable->next = block;
						pointerToEndOfDataTable = pointerToEndOfDataTable->next;
					}
				}

			}
			else
			{
				printf("Eror in file: %s row: %d apostrophes missing.\n", fullFileName, lineNumber);
				return false;
			}

			/*cheking for axcess character in the row*/
			row = row + strlen(data);
			for (i = 1; i < strlen(row); i++)
			{
				if ((row[i] != '\t') && (row[i] != ' ') && (row[i] != '\n') && (row[i] != EOF))
				{
					printf("Eror in file: %s row: %d there is an axcess character.\n", fullFileName, lineNumber);
					return false;
				}
			}
		}
		else
			return false;

		return true;
	}
	else/*it is .data*/
	{
		char number[MAX_ROW_LENGTH];
		int i = 0;
		int num;
		bool isNumExist = false;

		/*advencing row to the character after .data*/
		row = strstr(row, DATA) + strlen(DATA);


		while ((row[0] != '\n') && (row[0] != EOF))
		{
			/*ignor white spaces*/
			while ((row[0] == ' ') || (row[0] == '\t'))
				row = row + 1;

			/*find begining of number*/
			if ((row[0] == '+') || (row[0] == '-') || isdigit(row[0]))
			{
				number[i] = row[0];
				row = row + 1;
				i++;

				/*insert the digits to the buffer*/
				while (isdigit(row[0]))
				{
					number[i] = row[0];
					row = row + 1;
					i++;
				}

				/*check there is at least one digit*/
				if (isdigit(number[0]) == false && isdigit(number[1]) == false)
				{
					printf("Eror in file: %s row: %d  input is invalid.\n", fullFileName, lineNumber);
					return false;
				}
				/*convert to int*/
				num = atoi(number);
				/*mark as found at least one number*/
				isNumExist = true;

				/*end of number*/
				while ((row[0] != ',') && (row[0] != '\n'))
				{
					if (row[0] == '\t' || row[0] == ' ')
					{
						row = row + 1;
					}
					else/*non valide input*/
					{
						printf("Eror in file: %s row: %d  input is invalid.\n", fullFileName, lineNumber);
						return false;
					}
				}

				/*allocating memory and checking execution*/
				block = malloc(sizeof(codeLine));
				if (block == NULL)
				{
					printf("System eror in file: %s row: %d there was an memory alloction problem.\n", fullFileName, lineNumber);
					return false;
				}

				/*creating the memory block*/
				block->A = 1;
				block->R = 0;
				block->E = 0;
				block->last_bit = 0;
				block->dataWord.opcodeOrData = num;
				block->next = NULL;

				/*inserting the memory block to the data table*/
				if (dataTable == NULL)
					dataTable = block;
				else
				{
					pointerToEndOfDataTable->next = block;
					pointerToEndOfDataTable = pointerToEndOfDataTable->next;
				}
				DC++;	/*advencing deta counter */
			}
			else if (row[0] != '\n')
			{
				printf("Eror in file: %s row: %d  input is invalid.\n", fullFileName, lineNumber);
				return false;
			}
			else if (isNumExist == false) /*there was not numbers*/
			{
				printf("Eror in file: %s row: %d  input not found, missing numbers.\n", fullFileName, lineNumber);
				return false;
			}

			if (row[0] != '\n' && row[0] != EOF)/*reseting*/
			{
				i = 0;
				while (i < MAX_ROW_LENGTH)
				{
					number[i] = '\0';
					i++;
				}
				i = 0;
				row = row + 1;
			}
		}
		return true;
	}
}


void get_addresing_method(char* currentOperand, operand* operandCharacteristics)
{
	/*initializing operand data structure*/
	operandCharacteristics->addresing_mthod = -1;
	operandCharacteristics->immediate = 0;
	strcpy(operandCharacteristics->lable, "");
	operandCharacteristics->register_number = 0;

	if (currentOperand[0] == '#')/*is immediate*/
	{
		int i = 1;
		char immediate[MAX_ROW_LENGTH];
		/*if starts with + or - or digit*/
		if ((currentOperand[i] == '+') || (currentOperand[i] == '-') || isdigit(currentOperand[i]))
		{
			immediate[i - 1] = currentOperand[i];
			i++;
			/*insert all digits*/
			while (isdigit(currentOperand[i]))
			{
				immediate[i - 1] = currentOperand[i];
				i++;
			}

			/*go to end of current operand*/
			while (currentOperand[i] == '\t' || currentOperand[i] == ' ')
			{
				i++;
			}

			/*if there is illegal character*/
			if (currentOperand[i] != '\n' && currentOperand[i] != EOF && currentOperand[i] != '\0')
			{
				operandCharacteristics->addresing_mthod = -1;
				return;
			}
			else
			{
				operandCharacteristics->immediate = atoi(immediate);
				operandCharacteristics->addresing_mthod = 0;
				return;
			}
		}
	}
	else
	{
		int registerNumber;
		registerNumber = get_register_number(currentOperand);
		if (registerNumber != -1)/*is register direct*/
		{
			operandCharacteristics->addresing_mthod = 3;
			operandCharacteristics->register_number = registerNumber;
			return;
		}
		else
		{
			if (is_label(currentOperand) == true)/*is direct*/
			{
				operandCharacteristics->addresing_mthod = 1;
				strcpy(operandCharacteristics->lable, currentOperand);
				return;
			}

			/*checking thet the last character is ']'*/
			if (currentOperand[strlen(currentOperand) - 1] == ']')
			{
				char labelPart[MAX_ROW_LENGTH];
				int labelCounter = 0;
				char registerPart[MAX_ROW_LENGTH];
				int regCounter = 0;
				int i = 0;
				bool sowParenthesis = false;
				int registerNumber;

				/*breaking the operand to its label and its register part*/
				while (currentOperand[i] != ']')
				{
					if ((sowParenthesis == false) && (currentOperand[i] != '['))
					{
						labelPart[labelCounter] = currentOperand[i];
						labelCounter++;
						i++;
					}
					else if ((sowParenthesis == false) && (currentOperand[i] == '['))
					{
						labelPart[labelCounter] = '\0';
						sowParenthesis = true;
						i++;
					}
					else if (sowParenthesis == true)
					{
						registerPart[regCounter] = currentOperand[i];
						regCounter++;
						i++;
					}
				}
				registerPart[regCounter] = '\0';

				/*geting the register number*/
				registerNumber = get_register_number(registerPart);

				/*checking that the label is valid and the register is legal by asembler rules*/
				if ((is_label(labelPart) == true) && (registerNumber != -1) && (registerNumber >= 10) && (registerNumber <= 15))
				{
					operandCharacteristics->addresing_mthod = 2;
					strcpy(operandCharacteristics->lable, labelPart);
					operandCharacteristics->register_number = registerNumber;
					return;
				}
				else
				{
					operandCharacteristics->addresing_mthod = -1;
					return;
				}
			}
			else
			{
				operandCharacteristics->addresing_mthod = -1;
				return;
			}
		}
	}
	return;
}


bool add_funct_word(int funct, int register_dest, int register_source, int addres_dest, int addres_source, int rowNumber)
{
	/*funct block*/
	codeLine* block2;
	block2 = malloc(sizeof(codeLine));
	if (block2 != NULL)
	{
		block2->A = 1;
		block2->R = 0;
		block2->E = 0;
		block2->last_bit = 0;
		block2->dataWord.addresWord.addres_source = addres_source;
		block2->dataWord.addresWord.register_source = register_source;
		block2->dataWord.addresWord.addres_dest = addres_dest;
		block2->dataWord.addresWord.register_dest = register_dest;
		if (funct == -1)
			block2->dataWord.addresWord.funct = 0;
		else
			block2->dataWord.addresWord.funct = funct;

		block2->next = NULL;

		pointerToEndOfInstructionTable->next = block2;
		pointerToEndOfInstructionTable = block2;

		IC++;
	}
	else
	{
		printf("System eror in file: %s row: %d there was an memory alloction problem.\n", fullFileName, rowNumber);
		return false;
	}
	return true;
}


bool add_extra_words(operand operandCharacteristics, int rowNumber)
{
	bool retVal = true;

	switch (operandCharacteristics.addresing_mthod)
	{
	case 0:
	{
		/*immediat block*/
		codeLine* block3;
		block3 = malloc(sizeof(codeLine));
		if (block3 != NULL)
		{
			block3->A = 1;
			block3->R = 0;
			block3->E = 0;
			block3->last_bit = 0;
			block3->dataWord.opcodeOrData = operandCharacteristics.immediate;
			block3->next = NULL;

			pointerToEndOfInstructionTable->next = block3;
			pointerToEndOfInstructionTable = block3;

			IC++;

			retVal = true;
		}
		else
		{
			printf("System eror in file: %s row: %d there was an memory alloction problem.\n", fullFileName, rowNumber);
			retVal = false;
		}

		break;
	}
	case 1:
	case 2:
	{
		/*empty block for address base*/
		codeLine* block3;
		block3 = malloc(sizeof(codeLine));
		if (block3 != NULL)
		{
			block3->A = 0;
			block3->R = 0;
			block3->E = 0;
			block3->last_bit = 0;
			block3->dataWord.opcodeOrData = 0;
			block3->next = NULL;
			pointerToEndOfInstructionTable->next = block3;
			pointerToEndOfInstructionTable = block3;

			IC++;
		}
		else
		{
			printf("System eror in file: %s row: %d there was an memory alloction problem.\n", fullFileName, rowNumber);
			retVal = false;
		}
		if (retVal == true)
		{
			/*empty block for address offset*/
			codeLine* block4;
			block4 = malloc(sizeof(codeLine));
			if (block4 != NULL)
			{
				block4->A = 0;
				block4->R = 0;
				block4->E = 0;
				block4->last_bit = 0;
				block4->dataWord.opcodeOrData = 0;
				block4->next = NULL;
				pointerToEndOfInstructionTable->next = block4;
				pointerToEndOfInstructionTable = block4;

				IC++;
			}
			else
			{
				printf("System eror in file: %s row: %d there was an memory alloction problem.\n", fullFileName, rowNumber);
				retVal = false;
			}
		}

		break;
	}
	case 3:
	{
		break;
	}
	}
	return retVal;
}


bool fill_missing_word_data(operand operandCharacteristics, int rowNumber, FILE* externalsFaile)
{
	bool retVal = true;

	switch (operandCharacteristics.addresing_mthod)
	{
	case 0:
	{
		IC++;
		ptrIC = ptrIC->next;
		retVal = true;
		break;
	}
	case 1:
	case 2:
	{
		/*empty block for address base*/
		symbol* tempSymbol = is_symbol_exist(operandCharacteristics.lable);
		if (tempSymbol != NULL)
		{
			if (tempSymbol->isExternal == true)/*shoud mark as an external word*/
			{
				/*mark first word*/
				ptrIC->E = 1;
				fprintf(externalsFaile, "%s BASE %d \n", tempSymbol->name, IC);
				IC++;
				ptrIC = ptrIC->next;

				/*mark second word*/
				ptrIC->E = 1;
				fprintf(externalsFaile, "%s OFFSET %d \n\n", tempSymbol->name, IC);
				IC++;
				ptrIC = ptrIC->next;
			}
			else if (tempSymbol->isEntry || tempSymbol->isData || tempSymbol->isCode)/*shoud mark as anreferenc word*/
			{
				int base;
				int offset;

				get_base_addres_and_offset(tempSymbol->addres, &base, &offset);

				/*inserting the base of the addres*/
				ptrIC->R = 1;
				ptrIC->dataWord.opcodeOrData = base;
				IC++;
				ptrIC = ptrIC->next;

				/*inserting the offset of the addres*/
				ptrIC->R = 1;
				ptrIC->dataWord.opcodeOrData = offset;
				IC++;
				ptrIC = ptrIC->next;

			}
		}
		else
		{
			printf("Eror in file: %s row: %d could not find symbol - %s -.\n", fullFileName, rowNumber, operandCharacteristics.lable);
			retVal = false;
		}

		break;
	}
	case 3:
	{
		break;
	}
	}
	return retVal;
}


bool add_instruction(char* row, opcode_table* ptrCommand, int rowNumber)
{

	bool retVal = true;
	/*opcode block*/
	codeLine* block1;
	block1 = malloc(sizeof(codeLine));
	if (block1 != NULL)
	{
		block1->A = 1;
		block1->R = 0;
		block1->E = 0;
		block1->last_bit = 0;
		block1->dataWord.opcodeOrData = ptrCommand->opcode;
		block1->next = NULL;

		if (instructionTable == NULL)
		{
			instructionTable = block1;
			pointerToEndOfInstructionTable = block1;
		}
		else
		{
			pointerToEndOfInstructionTable->next = block1;
			pointerToEndOfInstructionTable = block1;
		}
		IC++;
	}
	else
	{
		printf("System eror in file: %s row: %d there was an memory alloction problem.\n", fullFileName, rowNumber);
		return false;
	}

	switch (ptrCommand->number_of_operands)
	{
	case 0:
	{
		break;
	}
	case 1:
	{
		operand operandCharacteristics;
		char currentOperand[MAX_ROW_LENGTH];

		/*read operand*/
		if (getNextWord(currentOperand, &row, MAX_ROW_LENGTH) == false)
		{
			printf("Eror in file: %s row: %d the operand dose not exist.\n", fullFileName, rowNumber);
			retVal = false;
		}

		/*get addresing method of the operand and characteristics*/
		get_addresing_method(currentOperand, &operandCharacteristics);

		/*if is not an addresing method*/
		if (operandCharacteristics.addresing_mthod == -1)
		{
			printf("Eror in file: %s row: %d incorrect operand.\n", fullFileName, rowNumber);
			retVal = false;
		}

		/*if the addresing method is valid for this command*/
		if (ptrCommand->addresing_mthod_dest_operand[operandCharacteristics.addresing_mthod] == false)
		{
			printf("Eror in file: %s row: %d the operand addresing method is incorrect.\n", fullFileName, rowNumber);
			retVal = false;
		}

		/*adding the funct word*/
		if (add_funct_word(ptrCommand->funct,
			operandCharacteristics.register_number,
			0,
			operandCharacteristics.addresing_mthod,
			0,
			rowNumber) == false)
			retVal = false;

		if (add_extra_words(operandCharacteristics, rowNumber) != true)
			retVal = false;

		break;
	}
	case 2:
	{
		operand firstOperandCharacteristics;
		char firstOperand[MAX_ROW_LENGTH];
		operand secondOperandCharacteristics;
		char secondOperand[MAX_ROW_LENGTH];

		/*read first operand*/
		if (getNextWord(firstOperand, &row, MAX_ROW_LENGTH) == false)
		{
			printf("Eror in file: %s row: %d the first operand dose not exist.\n", fullFileName, rowNumber);
			retVal = false;
		}
		/*if we read "," linked to the first word*/
		if (firstOperand[strlen(firstOperand) - 1] == ',')
		{
			firstOperand[strlen(firstOperand) - 1] = '\0';

			if (getNextWord(secondOperand, &row, MAX_ROW_LENGTH) == false)
			{
				printf("Eror in file: %s row: %d the first operand dose not exist.\n", fullFileName, rowNumber);
				retVal = false;
			}
		}
		else/*we read just the first word or the first + "," + the second */
		{
			char* temp = strstr(firstOperand, ",");
			if (temp != NULL) /*we read the first + "," + the second*/
			{
				strcpy(secondOperand, temp + 1);
				*temp = '\0';
			}
			else /*we read just the first word*/
			{
				if (getNextWord(secondOperand, &row, MAX_ROW_LENGTH) == false)
				{
					printf("Eror in file: %s row: %d the second operand dose not exist.\n", fullFileName, rowNumber);
					retVal = false;
				}

				if (strlen(secondOperand) == 1 && secondOperand[0] == ',')
				{
					if (getNextWord(secondOperand, &row, MAX_ROW_LENGTH) == false)
					{
						printf("Eror in file: %s row: %d the second operand dose not exist.\n", fullFileName, rowNumber);
						retVal = false;
					}
				}
				else
				{
					if (secondOperand[0] != ',')
					{
						printf("Eror in file: %s row: %d there is no comma between operands.\n", fullFileName, rowNumber);
						retVal = false;
					}
					else
					{
						strcpy(secondOperand, &secondOperand[1]);
					}
				}
			}
		}

		/*get addresing method of the operand and characteristics*/
		get_addresing_method(firstOperand, &firstOperandCharacteristics);

		/*if is not an addresing method*/
		if (firstOperandCharacteristics.addresing_mthod == -1)
		{
			printf("Eror in file: %s row: %d the first operand is incorrect.\n", fullFileName, rowNumber);
			retVal = false;
		}

		/*if not a valid addresing method for this command*/
		if (ptrCommand->addresing_mthod_src_operand[firstOperandCharacteristics.addresing_mthod] == false)
		{
			printf("Eror in file: %s row: %d the first operand addresing mthod is incorrect.\n", fullFileName, rowNumber);
			retVal = false;
		}

		get_addresing_method(secondOperand, &secondOperandCharacteristics);
		if (secondOperandCharacteristics.addresing_mthod == -1)
		{
			printf("Eror in file: %s row: %d the second operand is incorrect.\n", fullFileName, rowNumber);
			retVal = false;
		}

		if (ptrCommand->addresing_mthod_dest_operand[secondOperandCharacteristics.addresing_mthod] == false)
		{
			printf("Eror in file: %s row: %d the second operand addresing mthod is incorrect.\n", fullFileName, rowNumber);
			retVal = false;
		}

		/*adding the funct word*/
		if (add_funct_word(ptrCommand->funct,
			secondOperandCharacteristics.register_number,
			firstOperandCharacteristics.register_number,
			secondOperandCharacteristics.addresing_mthod,
			firstOperandCharacteristics.addresing_mthod,
			rowNumber) == false)
		{
			retVal = false;
		}

		/*adding the extra words for the first operand*/
		if (add_extra_words(firstOperandCharacteristics, rowNumber) == false)
		{
			retVal = false;
		}

		/*adding the extra words for the second operand*/
		if (add_extra_words(secondOperandCharacteristics, rowNumber) == false)
		{
			retVal = false;
		}

		break;
	}
	default:
	{
		printf("Eror in file: %s row: %d incorrect command.\n", fullFileName, rowNumber);
		retVal = false;
		break;
	}

	}
	/*check if row is clean after command for all cases*/
	char endOfRowChack[MAX_ROW_LENGTH];
	getNextWord(endOfRowChack, &row, MAX_ROW_LENGTH);
	if (endOfRowChack[0] != '\n' && endOfRowChack[0] != EOF && endOfRowChack[0] != '\0')
	{
		printf("Eror in file: %s row: %d there is an axcess character.\n", fullFileName, rowNumber);
		retVal = false;
	}
	return retVal;
}


bool fill_missing_addreses(char* row, opcode_table* ptrCommand, int rowNumber, FILE* externalsFaile)
{
	IC++; /*opcode*/
	ptrIC = ptrIC->next;

	switch (ptrCommand->number_of_operands)
	{
	case 0:
	{
		return true;
		break;
	}
	case 1:
	{
		operand operandCharacteristics;
		char currentOperand[MAX_ROW_LENGTH];

		/*read operand*/
		if (getNextWord(currentOperand, &row, MAX_ROW_LENGTH) == false)
		{
			printf("Eror in file: %s row: %d the first operand dose not exist.\n", fullFileName, rowNumber);
			return false;
		}

		/*get addresing method of the operand and characteristics*/
		get_addresing_method(currentOperand, &operandCharacteristics);

		/*adding the funct word*/
		IC++;
		ptrIC = ptrIC->next;

		if (fill_missing_word_data(operandCharacteristics, rowNumber, externalsFaile) == true)
			return true;
		else
			return false;

		break;
	}
	case 2:
	{
		operand firstOperandCharacteristics;
		char firstOperand[MAX_ROW_LENGTH];
		operand secondOperandCharacteristics;
		char secondOperand[MAX_ROW_LENGTH];

		/*read first operand*/
		if (getNextWord(firstOperand, &row, MAX_ROW_LENGTH) == false)
		{
			printf("Eror in file: %s row: %d the first operand dose not exist.\n", fullFileName, rowNumber);
			return false;
		}
		/*if we read "," linked to the first word*/
		if (firstOperand[strlen(firstOperand) - 1] == ',')
		{
			firstOperand[strlen(firstOperand) - 1] = '\0';

			if (getNextWord(secondOperand, &row, MAX_ROW_LENGTH) == false)
			{
				printf("Eror in file: %s row: %d the first operand dose not exist.\n", fullFileName, rowNumber);
				return false;
			}
		}
		else/*we read just the first word or the first + "," + the second */
		{
			char* temp = strstr(firstOperand, ",");
			if (temp != NULL) /*we read the first + "," + the second*/
			{
				strcpy(secondOperand, temp + 1);
				*temp = '\0';
			}
			else /*we read just the first word*/
			{
				if (getNextWord(secondOperand, &row, MAX_ROW_LENGTH) == false)
				{
					printf("Eror in file: %s row: %d the second operand dose not exist.\n", fullFileName, rowNumber);
					return false;
				}

				if (strlen(secondOperand) == 1 && secondOperand[0] == ',')
				{
					if (getNextWord(secondOperand, &row, MAX_ROW_LENGTH) == false)
					{
						printf("Eror in file: %s row: %d the second operand dose not exist.\n", fullFileName, rowNumber);
						return false;
					}
				}
				else
				{
					if (secondOperand[0] != ',')
					{
						printf("Eror in file: %s row: %d there is no comma between operands.\n", fullFileName, rowNumber);
						return false;
					}
					else
					{
						strcpy(secondOperand, &secondOperand[1]);
					}
				}
			}
		}

		/*get addresing method of the first operand and characteristics*/
		get_addresing_method(firstOperand, &firstOperandCharacteristics);

		/*get addresing method of the second operand and characteristics*/
		get_addresing_method(secondOperand, &secondOperandCharacteristics);

		/*funct word*/
		IC++;
		ptrIC = ptrIC->next;

		/*filling the the empty words*/
		if (fill_missing_word_data(firstOperandCharacteristics, rowNumber, externalsFaile) == false)
		{
			return false;
		}

		/*filling the the empty words*/
		if (fill_missing_word_data(secondOperandCharacteristics, rowNumber, externalsFaile) == false)
		{
			return false;
		}

		return true;

		break;
	}
	default:
	{
		printf("Eror in file: %s row: %d incorrect command.\n", fullFileName, rowNumber);
		return false;
		break;
	}

	}
}

bool first_loop(FILE* file)
{
	int lineCounter = 0;

	char row[MAX_ROW_LENGTH];
	char firstWord[MAX_ROW_LENGTH];
	char secondWord[MAX_ROW_LENGTH];
	char tempWord[MAX_ROW_LENGTH];
	opcode_table* ptrCommand;
	int i;

	/*flag that arises when we found compilation error*/
	bool problemInCode = false;

	/*going to the start of the file*/
	if (fseek(file, 0, SEEK_SET) != 0)
	{
		printf("System eror in file: %s could not find the begining of .am the file.\n", fullFileName);
		return false;
	}
	/*for each row in the file*/
	while (fgets(row, MAX_ROW_LENGTH, file))
	{
		char* chptr = row;
		bool isLineLable;
		lineCounter++;

		/*geting the first word in the row*/
		if (getNextWord(firstWord, &chptr, MAX_ROW_LENGTH))
		{
			/*if is a comment*/
			if (firstWord[0] == COMMENT_MARK)
				continue;
			/* check if valid label and remove ':'*/
			if (firstWord[strlen(firstWord) - 1] == ':')
			{
				firstWord[strlen(firstWord) - 1] = '\0';
				isLineLable = is_label(firstWord);
			}
			else
			{
				isLineLable = false;
			}

			if (isLineLable == false)
			{
				strcpy(secondWord, firstWord);
			}
			else
			{
				/*geting the second word in the row*/
				if (getNextWord(secondWord, &chptr, MAX_ROW_LENGTH) == false)
				{
					printf("Eror in file: %s row: %d expected command or instruction after lable.\n", fullFileName, lineCounter);
					problemInCode = true;
					continue;
				}
			}

			/*if the second word is data type*/
			if ((strcmp(secondWord, DATA) == 0) || (strcmp(secondWord, STRING) == 0))
			{
				if (isLineLable == true)
				{
					symbol* pointer = is_symbol_exist(firstWord);
					if (pointer == NULL)
					{
						/*inserting symbol to the symbol table*/
						symbol* symbolBlock = malloc(sizeof(symbol));
						if (symbolBlock != NULL)
						{
							symbolBlock->isData = true;
							symbolBlock->isEntry = false;
							symbolBlock->isExternal = false;
							symbolBlock->isCode = false;
							symbolBlock->addres = DC;
							strcpy(symbolBlock->name, firstWord);
							symbolBlock->next = NULL;

							if (symbolTable == NULL)
								symbolTable = symbolBlock;
							else
							{
								symbolBlock->next = symbolTable;
								symbolTable = symbolBlock;
							}
						}
						else
						{
							return false;
						}
					}
					else /*label exist in symbolTable*/
					{
						printf("Eror infile: %s row: %d lable %s is duplicate.\n", fullFileName, lineCounter, firstWord);
						return false;
					}
				}
				if (insert_data_to_mem(row, secondWord, lineCounter) == false)
					return false;
			}
			else if ((strcmp(secondWord, ENTRY) == 0))/*if is entry ignor*/
			{
				continue;
			}
			else if ((strcmp(secondWord, EXTERN) == 0))/*is extern*/
			{
				/*get next word*/
				if (getNextWord(tempWord, &chptr, MAX_ROW_LENGTH) == false)
				{
					printf("Eror in file: %s row: %d there is extern without a lable.\n", fullFileName, lineCounter);
					problemInCode = true;
					continue;
				}
				else
				{
					symbol* pointer = is_symbol_exist(tempWord);
					if (pointer == NULL)/*symbol is not in symbol table*/
					{
						symbol* symbolBlock = malloc(sizeof(symbol));
						symbolBlock->addres = 0;
						symbolBlock->isCode = false;
						symbolBlock->isData = false;
						symbolBlock->isEntry = false;
						symbolBlock->isExternal = true;
						strcpy(symbolBlock->name, tempWord);
						symbolBlock->next = NULL;

						if (symbolTable == NULL)
						{
							symbolTable = symbolBlock;
						}
						else
						{
							symbolBlock->next = symbolTable;
							symbolTable = symbolBlock;
						}
					}
					else/*symbol is in symbol table*/
					{
						/*if it is not external*/
						if (pointer->isExternal == false)
						{
							printf("Eror in file: %s row: %d lable allrady declerd as non external.\n", fullFileName, lineCounter);
							problemInCode = true;
							continue;
						}
						else
						{
							continue;
						}
					}
				}

			}
			else/*instruction*/
			{
				if (isLineLable == true)
				{
					if (is_symbol_exist(firstWord) == NULL)
					{
						symbol* symbolBlock = malloc(sizeof(symbol));
						if (symbolBlock != NULL)
						{
							symbolBlock->addres = IC;
							symbolBlock->isCode = true;
							symbolBlock->isData = false;
							symbolBlock->isEntry = false;
							symbolBlock->isExternal = false;
							strcpy(symbolBlock->name, firstWord);
							symbolBlock->next = NULL;
						}
						if (symbolTable == NULL)
						{
							symbolTable = symbolBlock;
						}
						else
						{
							symbolBlock->next = symbolTable;
							symbolTable = symbolBlock;
						}
					}
					else
					{
						printf("Eror in file: %s row: %d lable allrady declerd as non external.\n", fullFileName, lineCounter);
						problemInCode = true;
						continue;
					}
				}
				/*find instruction */
				ptrCommand = NULL;
				for (i = 0; i < 16; i++)
				{
					if (strcmp(secondWord, opcode_data[i].command_name) == 0)
					{
						ptrCommand = &opcode_data[i];
						break;
					}
				}

				/*if it is not an valide instruction*/
				if (ptrCommand == NULL)
				{
					printf("Eror in file: %s row: %d incorrect instruction.\n", fullFileName, lineCounter);
					problemInCode = true;
					continue;
				}
				else
				{
					if (add_instruction(chptr, ptrCommand, lineCounter) == false)
						problemInCode = false;
				}
			}
		}
	}

	/*check for memory over flow*/
	if ((IC + DC) > 8191)
	{
		printf("Eror in file: %s memory over flow.\n", fullFileName);
		problemInCode = true;
	}

	/*saving the instruction counter and the data countr*/
	ICF = IC;
	DCF = DC;

	/*addvancing the addres of the data symbols by the instruction amount*/
	if (symbolTable != NULL)
	{
		symbol* ptr = symbolTable;
		while (ptr != NULL)
		{
			if (ptr->isData == true)
			{
				ptr->addres += IC;
			}
			ptr = ptr->next;
		}
	}

	if (problemInCode == true)
		return false;
	else
		return true;
}


bool second_loop(FILE* file, char* originalFileName)
{
	char row[MAX_ROW_LENGTH];
	char firstWord[MAX_ROW_LENGTH];
	char secondWord[MAX_ROW_LENGTH];
	FILE* externalsFaile;

	char* externalsFaileName = malloc(strlen(originalFileName) + 5);

	/*flag that arises when we found compilation error*/
	bool problemInCode = false;

	int lineCounter = 0;

	IC = BASE_IC_ADDRESS;
	ptrIC = instructionTable;

	/*going to the start of the file*/
	if (fseek(file, 0, SEEK_SET) != 0)
	{
		printf("System eror in file: %s could not find the begining of .am the file.\n", fullFileName);
		return false;
	}

	/*creating the external file name*/
	strcpy(externalsFaileName, originalFileName);
	strcat(externalsFaileName, ".ext");

	/*opening a new file for the external products */
	externalsFaile = fopen(externalsFaileName, "w+");

	if (externalsFaile == NULL)
	{
		printf("System eror in file: %s could not create externalse file.\n", fullFileName);
		return false;
	}


	/*for each row in the file*/
	while (fgets(row, MAX_ROW_LENGTH, file))
	{
		bool isLineLable;
		char* chptr = row;
		lineCounter++;


		/*geting the first word in the row*/
		if (getNextWord(firstWord, &chptr, MAX_ROW_LENGTH))
		{
			/*if is a comment*/
			if (firstWord[0] == COMMENT_MARK)
				continue;
			/* check if valid label and remove ':'*/
			if (firstWord[strlen(firstWord) - 1] == ':')
			{
				firstWord[strlen(firstWord) - 1] = '\0';
				isLineLable = is_label(firstWord);
			}
			else
			{
				isLineLable = false;
			}

			if (isLineLable == false)
			{
				strcpy(secondWord, firstWord);
			}
			else
			{
				/*geting the second word in the row*/
				if (getNextWord(secondWord, &chptr, MAX_ROW_LENGTH) == false)
				{
					printf("Eror in file: %s row: %d expected command or instruction after lable.\n", fullFileName, lineCounter);
					problemInCode = true;
					continue;
				}
			}

			/*if the second word is data type*/
			if ((strcmp(secondWord, DATA) == 0) || (strcmp(secondWord, STRING) == 0))
			{
				continue;
			}
			else if ((strcmp(secondWord, ENTRY) == 0))/*if is entry ignor*/
			{
				if (getNextWord(firstWord, &chptr, MAX_ROW_LENGTH) == true)
				{
					symbol* temp;
					temp = is_symbol_exist(firstWord);
					if (temp != NULL)
					{
						temp->isEntry = true;
					}
					else
					{
						printf("Eror in file: %s row: %d symbol could not be obtained.\n", fullFileName, lineCounter);
						problemInCode = true;
						continue;
					}
				}
				else
				{
					printf("Eror in file: %s row: %d expected lable.\n", fullFileName, lineCounter);
					problemInCode = true;
					continue;
				}
			}
			else if ((strcmp(secondWord, EXTERN) == 0))/*is extern*/
			{
				continue;
			}
			else/*instruction*/
			{
				/*finde instruction */
				opcode_table* ptrCommand = NULL;
				int i;
				for (i = 0; i < 16; i++)
				{
					if (strcmp(secondWord, opcode_data[i].command_name) == 0)
					{
						ptrCommand = &opcode_data[i];
						break;
					}
				}

				/*if it is not an valide instruction*/
				if (ptrCommand == NULL)
				{
					printf("Eror in file: %s row: %d incorrect instruction.\n", fullFileName, lineCounter);
					problemInCode = true;
					continue;
				}
				else
				{
					if (fill_missing_addreses(chptr, ptrCommand, lineCounter, externalsFaile) == false)
						problemInCode = false;
				}

			}
		}
	}

	/*if externals file is empty delete it*/
	fseek(externalsFaile, 0, SEEK_END);
	if (ftell(externalsFaile) == 0)
	{
		fclose(externalsFaile);
		remove(externalsFaileName);
	}
	else
	{
		fclose(externalsFaile);
	}

	/*free to name of file memory allocation*/
	free(externalsFaileName);

	if (problemInCode == false)
		return true;
	else
		return false;
}


bool create_product(char* fileName)
{
	symbol* ptrSymbolTable;
	int base = 0;
	int offset = 0;
	FILE* entrysFile;
	char* entrysFileName;
	FILE* objectFile;
	char* objectFileName;
	codeLine* ptrMemory;

	/*creating entry file name and opening the file*/
	entrysFileName = malloc(strlen(fileName) + 5);
	strcpy(entrysFileName, fileName);
	strcat(entrysFileName, ".ent");

	entrysFile = fopen(entrysFileName, "w+");

	/*if file dident open*/
	if (entrysFile == NULL)
	{
		printf("System Eror: could not create entry file for: %s.\n", fullFileName);
		return false;
	}

	/*filling the entry file*/
	ptrSymbolTable = symbolTable;
	while (ptrSymbolTable != NULL)
	{
		if (ptrSymbolTable->isEntry == true)
		{
			get_base_addres_and_offset(ptrSymbolTable->addres, &base, &offset);
			fprintf(entrysFile, "%s,%d,%d\n", ptrSymbolTable->name, base, offset);
		}
		ptrSymbolTable = ptrSymbolTable->next;
	}

	/*if the entry file is empty remove it*/
	fseek(entrysFile, 0, SEEK_END);
	if (ftell(entrysFile) == 0)
	{
		fclose(entrysFile);
		remove(entrysFileName);
	}
	else
	{
		fclose(entrysFile);
	}
	/*free the entry file name memory*/
	free(entrysFileName);


	/*creating object file name and opening the file*/
	objectFileName = malloc(strlen(fileName) + 5);
	strcpy(objectFileName, fileName);
	strcat(objectFileName, ".ob");

	objectFile = fopen(objectFileName, "w+");

	/*if file dident open*/
	if (objectFile == NULL)
	{
		printf("System Eror: could not create object file for: %s.\n", fullFileName);
		return false;
	}

	/*first row in the object file*/
	fprintf(objectFile, "%d %d\n", ICF - BASE_IC_ADDRESS, DCF);

	/*connecting the instruction table to the data table*/
	pointerToEndOfInstructionTable->next = dataTable;

	ptrMemory = instructionTable;

	IC = BASE_IC_ADDRESS;

	/*convarting the memory to to object format and inserting to .ob file*/
	while (ptrMemory != NULL)
	{
		printWord(objectFile, IC, ptrMemory);
		ptrMemory = ptrMemory->next;
		IC++;
	}

	fclose(objectFile);

	free(objectFileName);

	return true;

}


void reset_all(char* fullFileName)
{
	codeLine* cleanIC;
	symbol* cleanSymbol;

	/*file name*/
	free(fullFileName);

	/*global counters*/
	IC = BASE_IC_ADDRESS;
	DC = 0;
	ICF = 0;
	DCF = 0;

	/*instruction table and data table*/
	dataTable = NULL;
	cleanIC = instructionTable;
	while (instructionTable != NULL)
	{
		cleanIC = cleanIC->next;
		free(instructionTable);
		instructionTable = cleanIC;
	}

	/*symbol table*/
	cleanSymbol = symbolTable;
	while (symbolTable != NULL)
	{
		cleanSymbol = cleanSymbol->next;
		free(symbolTable);
		symbolTable = cleanSymbol;
	}
}

#pragma once
#ifndef _ASEMBLER_STRING 
#define _ASEMBLER_STRING

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ROW_LENGTH 81
#define COMMENT_MARK ';'
#define EXTERN ".extern"
#define DATA ".data"
#define STRING ".string"
#define ENTRY ".entry"
#define BASE_IC_ADDRESS 100

/*memory picture*/
typedef struct {
	unsigned int addres_dest : 2;
	unsigned int register_dest : 4;
	unsigned int addres_source : 2;
	unsigned int register_source : 4;
	unsigned int funct : 4;
}AddresWord;

typedef union {
	short int opcodeOrData;
	AddresWord addresWord;
}DataWord;

typedef struct codeLine {
	DataWord dataWord;
	unsigned int E : 1;
	unsigned int R : 1;
	unsigned int A : 1;
	unsigned int last_bit : 1;
	struct codeLine* next;
}codeLine;

/*the symbol table*/
typedef struct symbol {
	char name[MAX_ROW_LENGTH];
	int addres;
	bool isEntry;
	bool isData;
	bool isExternal;
	bool isCode;
	struct symbol* next;
}symbol;


/*opcodes and addresing methods*/
typedef struct {
	char command_name[5];
	int opcode;
	int funct;
	int number_of_operands;
	bool addresing_mthod_src_operand[4];
	bool addresing_mthod_dest_operand[4];
}opcode_table;

/*operand information*/
typedef struct operand {
	int addresing_mthod;
	int immediate;
	int register_number;
	char lable[MAX_ROW_LENGTH];
}operand;

opcode_table opcode_data[16];

/*start of declerations*/

/*the function check if given symbol is in the symbol table and returns pointer to the simboll found
else rturn null*/
symbol* is_symbol_exist(char* to_comper);

/*if lable given is a leagal one returns true else false*/
bool is_label(char* lable);

/*inserting the data to the data table / memory. if secsesful returns true else false */
bool insert_data_to_mem(char* row, char* dataType, int lineNumber);

/*fills the instruction table, data table and the symbol table. if successful returns true else false*/
bool first_loop(FILE* file);

/*fills the empty words in the instruction table, producing .ext file if needed*/
bool second_loop(FILE* file, char* originalFileName);

/*create the .ent file if needed and create the .ob file*/
bool create_product(char* fileName);

/*free all memory allocations and reseting globals*/
void reset_all(char* fullFileName);

/*manages the execution steps*/
void process_file(char* fileName);

/*gets operand if successful returns its addresing mthod else returns -1 (in operand data structure)*/
void get_addresing_method(char* currentOperand, operand* operandCharacteristics);

/*the function adds the opcode word and the funct word to the instruction table. return true if successful*/
bool add_funct_word(int funct, int register_dest, int register_source, int addres_dest, int addres_source, int rowNumber);

/*adding to instruction table an instruction extra words. if successful returns true else false*/
bool add_extra_words(operand operandCharacteristics, int rowNumber);

/*inserting instruction to the instruction table. if successful returns true else false*/
bool add_instruction(char* row, opcode_table* ptrCommand, int rowNumber);

/*fill in the empty words in the instruction table end fill the .ext file when needed.
if successful returns true else false*/
bool fill_missing_word_data(operand operandCharacteristics, int rowNumber, FILE* externalsFaile);

/*fill the empty words in the instruction table. if successful returns true else false*/
bool fill_missing_addreses(char* row, opcode_table* ptrCommand, int rowNumber, FILE* externalsFaile);


/*end of declerations*/


#endif /*_ASEMBLER_STRING*/
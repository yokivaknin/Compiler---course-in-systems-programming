#include "asemblerUtils.h"

/*macro content*/
typedef struct macro_content
{
	char macro_row_content [MAX_ROW_LENGTH];
	struct macro_content* next;

}macro_content;

/*macro table*/
typedef struct macro_list {

	char name [MAX_ROW_LENGTH];
	macro_content* content;
	struct macro_list* next;

}macro_list;

/*start of declaration*/

FILE* process_macro(FILE* current, char* cleanFileName);

bool is_valid_name(char* word);

int get_register_number(char* word);

/*end of declaration*/
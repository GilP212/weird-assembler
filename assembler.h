#define MAXLABEL 31 /*Maximum number of chars in a label.*/
#define MAXTABLE 260 /*Maximum lines in text file.*/
#define MAXLINE 81 /*Maximum length of each line.*/
#define MAXNUM 20 /*Maximum digits in a number operand.*/
#define NOADDRESS -1 /*Not a possible address, used to signal an address is not defined.*/
#define MAXFILENAME 100 /*Maximum amount of characters in a filename.*/
#define MEMORYSTART 100 /*Starting address of memory.*/

#define TRUE 1
#define FALSE 0

/*A- Absolute, R- Relocatable, E- External.*/
enum ARE_FIELD {A, E, R};

/*These are the various error ids the program encounters, proper response printed to stderr.*/
enum ERR_STATUS {INVALID = 0, VALID = 1, COMMENT_LINE = 2, EMPTY_LINE = 3, ERR_NEW_LINE, ERR_FIRST_NOT_ALPHA, ERR_NOT_ALNUM, ERR_UNKNOWN_CMD, ERR_UNKNOWN_DIR, ERR_LABEL_SAVED_WORD, ERR_NO_ENTRY_EXISTS, ERR_DATA_OPERANDS, ERR_STRING_OPERANDS, ERR_STRUCT_OPERANDS, ERR_WRONG_NUM_OPS, ERR_SYMBOL_NOT_FOUND, ERR_FAILED_FILE_OPEN, ERR_FAILED_FILE_REMOVE, ERR_UNKNOWN_REGISTER};

/*A total of 21 different line types, either a command or a directive.*/
enum LINE_TYPES {MOV = 0, CMP, ADD, SUB, NOT, CLR, LEA, INC, DEC, JMP, BNE, RED, PRN, JSR, RTS, STOP, ENT, EXT, DATA, STR, STRUCT};

/*the extline type is used as a reference to a line of text containing an external symbol.
name - the external symbol's label. lineAddress - the line in which it appears.*/
typedef struct {
	char *name;
	int *lineAddress;
} extline;

/*symboltable is used to store info about various symbol declarations in the source file.*/
typedef struct {
	char name[MAXLABEL]; /*Label.*/
	short int address; /*Address of line reffered to by symbol.*/
	unsigned int is_cmd : 1; /*1 means command, 0 means data. NEED2FIX: add enums of CMD = 1, DATA = 0*/
	unsigned int is_ext : 1; /*is external symbol.*/
	unsigned int is_ent : 1; /*is entry symbol.*/
} symboltable[MAXTABLE];

/*table is used to store a line of code in binary form, and it's address in the cmd/data tables.*/
typedef struct {
	int address; 
	int code; 
} table[MAXTABLE];




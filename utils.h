#define NUMOFCMDS 16 /*Total number of different commands.*/
#define NUMOFDIRS 5 /*Total number of different directives.*/

/*macro that returns first 2 ARE bits of a cmd code line.*/
#define ARE_MASK(num) (num & 3) /*3 == 0000000011*/

/*Text line parsing macros.*/
#define skip_space(str, i) \
	for(; isspace(str[i]); i++)

#define skip_non_space(str, i) \
	for(; !isspace(str[i]); i++)

/*Diefferent bit positions in code lines.*/
enum BITPOS {DESTBITS = 2, SOURCEBITS = 4, CMDBITS = 6, OPBITS = 2, SOURCEREG = 6, DESTREG = 2, SECOND_HALF_BITS = 5};

/*FUNCTION PROTOTYPES*/
/* Recieves char string, returns an enum LINE_TYPES integer of saved word identical to string recieved, else returns NUMOFCMDS + NUMOFDIRS.*/
int isSavedWord(char *);

/*Creates .ob and .ent output files, returns VALID if successful. Receives as paramaters the file name, different table counters and the table addresses themselves.*/
int createOutput(char *fileName, int IC, int DC, int SC, symboltable symTable, table dataTable, table cmdTable);

/*Receives file name, name of external symbol reference and line where it appears. Prints to .ext file.*/
void outputToExt(int address, char *name, char *fileName);

/*Adds file ending to given origin string, puts in dest. Returns dest.*/
char *addFileEnd(char *dest, char *origin,char *ending);

/*Receives symbol table, data table and table counters. Updates addresses of data and symbol tables according to IC.*/
void updateTablesAddress(symboltable symTable, table dataTable, table cmdTable, int IC, int DC, int SC);

/*Receives a command number, returns number of required operands by specific command.*/
int numOfOperands(int cmdNum);

/*Recieves two integer pointers to store result. Finds first and second operands index in lineBuff.
May give NULL parameter as secondIndex if only one operand required. Function will store 0 in index if no operand found.*/
void getOperandIndex(int *firstIndex, int *secondIndex, char *lineBuff);

/*Recieves a line string, returns index of second operand, skipping the first.*/
int secondOperandIndex(char *lineBuff);




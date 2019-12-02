
/*PROTOTYPES: symbol_table.c*/

/*Recieves a label, address in data table, a symbol table counter and symbol table. If label[0] != '\0', the label is stored in the symbol table with all info and all flags set to FALSE.*/
void insertDataSymbol(char *label, int DC, int SC, symboltable symTable);

/*Recieves a label, address in command table, a symbol table counter and symbol table. Works similar to insertDataSymbol, flags set: is_cmd = TRUE, is_ext = is_ent = FALSE. */
void insertCmdSymbol(char *label, int IC, int SC, symboltable symTable);

/*Recieves a line identified as EXT (enum LINE_TYPES), symbol table counter and symbol table. Reads line for the external symbol label, stores symbol info in symbol table. Flags set is_ext = TRUE, is_ent = is_cmd = FALSE. Address set to NOADDRESS.*/
void insertExternSymbol(char *lineBuff, int SC, symboltable symTable);

/*Recieves a symbol name, symbol table, symbol table counter and index to store address of symbol in symbol table, if one exists. Checks if given label exists. if it does, returns symbol address and stores in index, else returns NOADDRESS.*/
int findSymbolAddress(char *symbolName, symboltable symTable, int SC, int *index);

/*Recieves command table counter, symbol table counter and symbol table. Adds value of MEMORYSTART to all symbols where (is_ext == FALSE), and also value of IC to all symbols where (is_cmd == FALSE).*/
void updateSymbolTableData(int IC, int SC, symboltable symTable);

/*Recieves a line identified as ENT (enum LINE_TYPES), symbol table and symbol table counter. If entry symbol exists in symbol table, updates flag is_int = TRUE. Else returns error.*/
int updateEntrySymbol(char *lineBuff, symboltable symTable, int SC);


/*PROTOTYPES: data_table*/

/*Recieves a line identified as DATA (enum LINE_TYPES), data table counter and data table. Reads integer operands from line, stores value and address for each operand and updates DC. Returns enum ERR_STATUS type.*/
int insertDataTable(char *lineBuff, int *DC, table dataTable);

/*Recieves a line identified as STR (enum LINE_TYPES), data table counter and data table. Reads a string of chars from line, stores value and address for each char including ending '\0' and updates DC. Returns enum ERR_STATUS type.*/
int insertStringTable(char *lineBuff, int *DC, table dataTable);

/*Recieves a line identified as STRUCT (enum LINE_TYPES), data table counter and data table. Reads one integer followed by one string operand from line, stores value and address in data table and updates DC. Returns enum ERR_STATUS type.*/
int insertStructTable(char *, int *, table );

/*PROTOTYPES: cmd_table*/

/*Recieves a line identified as a command (enum LINE_TYPES), command table counter, number of command (enum LINE_TYPES) and command table. Translates line into first code word, stores in cmdTable at *IC and updates IC. Returns enum ERR_STATUS type.*/
int insertCmdTable(char *lineBuff, int *IC, int cmdNum, table cmdTable);

/*Recieves the following paramaters: 
lineBuff- a line identified as a command. 
IC- pointer to the command table counter in "second scan".
cmdType- The command number (enum LINE_TYPES).
cmdTable- The commands table.
symTable- The symbols table.
SC- symbol table counter.
fileName- the name of the source file, with no file ending.
extSymbol- a buffer of type extline.
The function reads operands from the line, translates into the proper code words and stores in cmdTable. Also, will output any references to external symbols found. Returns enum ERR_STATUS type.*/
int completeCmdTable(char *lineBuff, int *IC, int cmdType, table cmdTable, symboltable symTable, int SC, char *fileName, extline extSymbol);




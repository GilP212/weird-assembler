#include "assembler.h"
#include "utils.h"
#include <string.h>
#include <ctype.h> 

void getExtEntSymbol(char *, char *);
int findSymbolAddress(char *, symboltable , int , int *);

/*insertDataSymbol checks if a label has been found, copies label and address (Data Counter position) into the symbol table and sets correct flags for a data symbol. This is a label defined before .data/.string/.struct directive.*/
void insertDataSymbol(char *label, int DC, int SC, symboltable symTable)
{
	if(label[0] != '\0')
	{
		strcpy(symTable[SC].name, label);
		symTable[SC].address = DC;
		symTable[SC].is_cmd = FALSE; /*it is data, not cmd.*/
		symTable[SC].is_ext = FALSE;
		symTable[SC].is_ent = FALSE;
	}
}

/*insertCmdSymbol works similar to insertDataSymbol, but for command type symbols.*/
void insertCmdSymbol(char *label, int IC, int SC, symboltable symTable)
{
	if(label[0] != '\0')
	{
		strcpy(symTable[SC].name, label);
		symTable[SC].address = IC;
		symTable[SC].is_cmd = TRUE;
		symTable[SC].is_ext = FALSE;
		symTable[SC].is_ent = FALSE;
	}

}

/*insertExternSymbol reads an external symbol declaration in an .extern directive line type, then stores label and sets flags. As an external symbol, address is irrelevant and set to NOADDRESS for future checks.*/
void insertExternSymbol(char *lineBuff, int SC, symboltable symTable)
{
	getExtEntSymbol(symTable[SC].name, lineBuff);
	symTable[SC].address = NOADDRESS;
	symTable[SC].is_cmd = FALSE; 
	symTable[SC].is_ext = TRUE; 
	symTable[SC].is_ent = FALSE;
}

/*updateEntrySymbol reads a .entry directive line, checks if the entry symbol exists in the symbol table and if it does updates flag.*/
int updateEntrySymbol(char *lineBuff, symboltable symTable, int SC)
{
	int index;
	char entBuff[MAXLABEL];

	getExtEntSymbol(entBuff, lineBuff);

	if(findSymbolAddress(entBuff, symTable, SC, &index) == NOADDRESS) /*An entry symbol must refer to an existing label.*/
		return ERR_NO_ENTRY_EXISTS;
	else
		symTable[index].is_ent = TRUE;

	return VALID;
}

/*updateSymbolTableData updates the address information of non-external symbols, assuming memory starts at MEMORYSTART. It also updates directive symbols by final IC value.*/
void updateSymbolTableData(int IC, int SC, symboltable symTable)
{
	int i;

	for(i = 0; i < SC; i++)
		if(symTable[i].is_ext == FALSE) /*must be non-extern data symbol*/
			symTable[i].address += ((symTable[i].is_cmd == TRUE) ? (MEMORYSTART) : (IC + MEMORYSTART));
}

/*Parses an .entry/.extern line directive and finds the label declared, stores into wordBuff with an ending '\0'.*/
void getExtEntSymbol(char *wordBuff, char *lineBuff) 
{
	int i, j = 0;

	for(i = 0; lineBuff[i] != '.'; i++) /*in main function, we already checked ".extern"/".entry" is in lineBuff. this way we skip useless label definitions at start of entry/extern sentence.*/
		;

	skip_non_space(lineBuff, i); /*skip the .extern/.entry token*/

	skip_space(lineBuff, i); /*skip white space until extern/entry label*/

	for(; !isspace(lineBuff[i]); i++)
		wordBuff[j++] = lineBuff[i];

	wordBuff[j] = '\0';
}

/*findSymbolAddress searches for a given symbol name in the symbol table. If the name exits, the address in the symbol table is returned and stored in an index. Else NOADDRESS is returned.*/
int findSymbolAddress(char *symbolName, symboltable symTable, int SC, int *index)
{
	int i;

	for(i = 0; (strcmp(symTable[i].name, symbolName)) && (i < SC); i++)
		;

	if(index)/*if NULL, user does not need index stored, just returned.*/
		*index = i;

	if(i == SC)
		return NOADDRESS; 
	else
		return (symTable[i].address);
}












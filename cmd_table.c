#include "assembler.h"
#include "tables.h"
#include "utils.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

/*This macro handles inserting a DIRECT or LISTING operand, that need to find their symbol in the symbol table.*/
#define HANDLESYMBOL \
		{\
			for(i = start; (lineBuff[i] != '.') && (lineBuff[i] != ',') && !isspace(lineBuff[i]); i++)\
				opBuffer[i - start] = lineBuff[i];\
			opBuffer[i - start] = '\0';\
			if((opData = findSymbolAddress(opBuffer, symTable, SC, &temp)) < 0){\
				if(symTable[temp].is_ext){\
					strcpy(extSymbol.name, symTable[temp].name);\
					*extSymbol.lineAddress = *IC + MEMORYSTART;}\
				else\
					return ERR_SYMBOL_NOT_FOUND;}\
			cmdTable[*IC].code = (opData << OPBITS);\
			cmdTable[*IC].code += ((symTable[temp].is_ext)? E : R);\
			cmdTable[*IC].address = *IC + MEMORYSTART;\
			*IC += 1;\
		}

/*firstWord sets a bitfield on a 10-bit code line of the first word in the command table.*/
typedef struct {
	unsigned int cmdCode: 4; /*The number of the code. (enum LINE_TYPES)*/
	unsigned int sourceOp: 2; /*The type of address source operand (enum ADDRESS_TYPES).*/
	unsigned int destOp: 2; /*The type of address destination operand (enum ADDRESS_TYPES).*/
	unsigned int are: 2; /*one of the enum ARE_FIELD types.*/
} firstword;

/*The type of address an operand can have.*/
enum ADDRESS_TYPES {INSTANT, DIRECT, LISTING, REGISTER};

/*Either destination/source operand.*/
enum OP_TYPE {DEST, SOURCE};

int getAddressType(int , char *);
int codeToNum(firstword );
void updateWordsNeeded(int *, firstword , int , int);
int insertOperand(int , int , int , int , int , char *, int *, table , extline, symboltable );
int typeFromFirstWord(int , int );

/*insertCmdTable reads a command type line and calculates the first word of code to be stored in the command table. Returns enum ERR_STATUS type.*/
int insertCmdTable(char *lineBuff, int *IC, int cmdNum, table cmdTable)
{
	firstword code; /*this is the line of code that will be input into cmdTable.*/
	int firstIndex = 0, secondIndex = 0;/**/
	
	code.cmdCode = cmdNum;
	code.are = A; /*first word is absolute.*/
	
	getOperandIndex(&firstIndex, &secondIndex, lineBuff); /*first index will be left 0 if only one index*/

	code.sourceOp = getAddressType(firstIndex, lineBuff);

	code.destOp = getAddressType(secondIndex, lineBuff);

	/*add code to cmdTable*/
	cmdTable[*IC].code = codeToNum(code);
	cmdTable[*IC].address = *IC;

	/*update IC */
	updateWordsNeeded(IC, code, firstIndex, secondIndex);

	return VALID;
}

/*completeCmdTable reads a command type line, extracting operands for error checking and storing in cmdTable. This function is part of the "second scan" of the assembler algorithm, assuming the symbolTable is now filled with all info needed to finish cmdTable. In case an external symbol is encounter, extSymbol is used as a buffer for the info to be output to the .ext file.*/
int completeCmdTable(char *lineBuff, int *IC, int cmdType, table cmdTable, symboltable symTable, int SC, char *fileName, extline extSymbol)
{
	int firstIndex, secondIndex, i; /*first and second indexes for cmd operands.*/
	int numOfOps = 0,  opType; /*numOfOps- how many operands successfully read. */
	int wasRegister = FALSE; /*Tracks if a register operand was enoucntered, as two reg operands share a word of code.*/
	int err = VALID; /*err- keeps track of errors (enum ERR_STATUS).*/
	int extAddress = NOADDRESS; /* extAddress- will hold address of any ext symbols found.*/

	extSymbol.lineAddress = &extAddress;
	i = *IC; /*to save location of first code word*/
	*IC += 1; /*first code word is already there*/

	getOperandIndex(&firstIndex, &secondIndex, lineBuff);

	if((firstIndex) && (secondIndex))
		numOfOps += 2;
	else if(secondIndex)
		numOfOps += 1;

	if(numOfOps != numOfOperands(cmdType)) /*check if correct amount of operands given.*/
		return ERR_WRONG_NUM_OPS;

	if(!numOfOps) /*No operands, so no extra words needed besides first word.*/
		return VALID;

	opType = typeFromFirstWord(cmdTable[i].code, DEST);
	err = insertOperand(SC, DEST, wasRegister, opType, secondIndex, lineBuff, IC, cmdTable, extSymbol, symTable);

	if(opType == REGISTER) /*2 register operands share same code word.*/
		wasRegister++;

	if((*extSymbol.lineAddress >= 0) && (err == VALID))
	{
		outputToExt(*extSymbol.lineAddress, extSymbol.name, fileName);
		*extSymbol.lineAddress = NOADDRESS;
	}
	
	if(numOfOps == 2 && (err == VALID))
	{
		opType = typeFromFirstWord(cmdTable[i].code, SOURCE);
		err = insertOperand(SC, SOURCE, wasRegister, opType, firstIndex, lineBuff, IC, cmdTable, extSymbol, symTable);
		if((*extSymbol.lineAddress > NOADDRESS) && (err == VALID)) /*in case an external symbol was found, it is output to the .ext file for future reference.*/
		{
			outputToExt(*extSymbol.lineAddress, extSymbol.name, fileName); 
			*extSymbol.lineAddress = NOADDRESS;
		}
	}

	return err;
}

/*getAddressType reads an operand at start index, returns which on of enum ADDRESS_TYPES it is.*/
int getAddressType(int start, char *lineBuff)
{
	int i = start;
	char c; 

	if(!start)
		return 0;

	if((c = lineBuff[i]) == '#') /*instant operand must start with '#' char.*/
		return INSTANT;

	if(c == 'r') /*Register starts with 'r' char, followed by integer.*/
		return REGISTER;

	for(; (lineBuff[i] != '.') && (!isspace(lineBuff[i])) && (lineBuff[i] != '\0'); i++)
		;

	if(lineBuff[i] == '.') /*Listing has '.' char to seperate field number.*/
		return LISTING;
	else/*If none of the above, must be symbol or unknown symbol.*/
		return DIRECT;
		
}

/*codeToNum takes a firstword type of code, and converts it into a binary number.*/
int codeToNum(firstword code)
{
	int result;
	unsigned int temp;

	result = code.are;

	temp = code.destOp;
	result += temp << DESTBITS;
	
	temp = code.sourceOp;
	result += temp << SOURCEBITS;

	temp = code.cmdCode;
	result += temp << CMDBITS;

	return result;
}

/*updateWordsNeeded updates IC by number of code words needed, by inspecting the types of operands given. */
void updateWordsNeeded(int *IC, firstword code, int firstOp, int secondOp)
{
	*IC += 1; /*the first code word is mandatory.*/

	if(secondOp)
		switch(code.destOp)
		{
			case INSTANT: 
			case DIRECT: 
			case REGISTER: *IC += 1;
				break;
			case LISTING: *IC += 2;
				break;
			default: 
				break;
		}

	if(firstOp) /*will be 0 if only one operand exists.*/
		switch(code.sourceOp)
		{
			case INSTANT: 
			case DIRECT:  *IC += 1;
				break;
			case REGISTER:
				if(code.destOp != REGISTER) /*Two register operands share the same word of code.*/
					*IC += 1;
				break;
			case LISTING: *IC += 2;
				break;
			default: 
				break;
		}
}

/*insertOperand handles each case of enum ADDRESS_TYPES in the completCmdTable() function.*/
int insertOperand(int SC, int whichOp, int wasRegister, int opType, int start, char *lineBuff, int *IC, table cmdTable, extline extSymbol, symboltable symTable)
{
	int i, opData, temp = 0;
	char opBuffer[MAXNUM];

	*extSymbol.lineAddress = NOADDRESS;

	switch(opType)
	{
		case INSTANT:
			for(i = start + 1; (lineBuff[i] != ',') && !isspace(lineBuff[i]); i++)
				opBuffer[i - start - 1] = lineBuff[i];
			opBuffer[i - start - 1] = '\0';
			cmdTable[*IC].code = atoi(opBuffer) << OPBITS;
			cmdTable[*IC].address = *IC + MEMORYSTART;
			*IC += 1;
			break;
		case DIRECT:
				HANDLESYMBOL;
			break;
		case LISTING:
			HANDLESYMBOL;
			cmdTable[*IC].code = atoi(&(lineBuff[i + 1])) << OPBITS;
			cmdTable[*IC].code += R;
			cmdTable[*IC].address = *IC + MEMORYSTART;
			*IC += 1;
			break;
		case REGISTER: 
			if(wasRegister)
				*IC -= 1;
			else
				cmdTable[*IC].code = 0;

			temp = atoi(lineBuff + start + 1);
			if(temp < 0 || temp > 7)
				return ERR_UNKNOWN_REGISTER;
			else if(whichOp == DEST)
				temp = temp << DESTREG; 
			else
				temp = temp << SOURCEREG;

			cmdTable[*IC].code += temp;
			cmdTable[*IC].address = *IC + MEMORYSTART;
			*IC += 1;
			break;
	}

		
	return VALID;
}

/*typeFromFirstWord checks the first word of code of a command and determines type of operand set during insertCmdTable(). */
int typeFromFirstWord(int word, int opNum)
{
	int mask; /*mask used to copy operand bits.*/

	if(opNum == DEST)
	{
		mask = 3 << DESTBITS; /*mask == 0000001100*/
		mask = mask & word;
		mask = mask >> DESTBITS;
	}
	else if(opNum == SOURCE)
	{
		mask = 3 << SOURCEBITS; /*mask == 0000110000*/
		mask = mask & word;
		mask = mask >> SOURCEBITS;
	}

	return mask;
}









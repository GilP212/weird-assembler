#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "assembler.h"
#include "tables.h"
#include "utils.h"

#define MAXDIGITS 3 /*Max digits in weird 32 base number.*/

void createObjFile(int , int , table , table , FILE *);
void createEntFile(symboltable , int , FILE *);
char *convertTo32Base(int , char *);

/*createOutput function creates .ob and .ent files, according to the described format in the exercise. Opens and closes file pointers in write-only mode. Returns VALID/INVALID depending on success of file actions.*/
int createOutput(char *fileName, int IC, int DC, int SC, symboltable symTable, table dataTable, table cmdTable)
{
	FILE *objFP, *entFP; /*FILE pointers to .ob and .ent files.*/
	char obFileName[MAXFILENAME], entFileName[MAXFILENAME]; /*Buffers for full file names.*/

	objFP = fopen(addFileEnd(obFileName, fileName, ".ob"), "w");
	entFP = fopen(addFileEnd(entFileName, fileName, ".ent"), "w");

	if(!objFP || !entFP) /*Failure if either file failed to open.*/
		return INVALID;

	createObjFile(IC, DC, dataTable, cmdTable, objFP);
	createEntFile(symTable, SC, entFP);

	if(fclose(objFP) == EOF || fclose(entFP) == EOF)
		return INVALID;

	return VALID;
}

/*Prints address and line of code for each line in cmd_table and data_table, in that order, to objFP. The lines are printed in weird 32 base.*/
void createObjFile(int IC, int DC, table dataTable, table cmdTable, FILE *objFP)
{
	int i;
	char buffer1[MAXDIGITS], buffer2[MAXDIGITS]; /*buffers used to hold the address and line of code, respectively*/

	/*print first line*/
	fprintf(objFP, "\t%s\t%s \n", convertTo32Base(IC, buffer1), convertTo32Base(DC, buffer2));
	
	/*print commands*/
	for(i = 0; i < IC; i++)
		if(ARE_MASK(cmdTable[i].code) != E)
			fprintf(objFP, "%s\t%s \n", convertTo32Base(cmdTable[i].address, buffer1), convertTo32Base(cmdTable[i].code, buffer2));
		else
			fprintf(objFP, "%s\t%s \n", convertTo32Base(cmdTable[i].address, buffer1), convertTo32Base(0, buffer2)); /*if external, address should be 0.*/
			

	/*print data*/
	for(i = 0; i < DC; i++)
		fprintf(objFP, "%s\t%s \n", convertTo32Base(dataTable[i].address, buffer1), convertTo32Base(dataTable[i].code, buffer2));
}

/*Prints the label and address of each line declared as an entry to entFP.*/
void createEntFile(symboltable symTable, int SC, FILE *entFP)
{
	int i;
	char buffer[MAXDIGITS]; /*buffer used to hold the line address.*/

	for(i = 0; i < SC; i++)
		if(symTable[i].is_ent)
			fprintf(entFP, "%s\t%s \n", symTable[i].name, convertTo32Base(symTable[i].address, buffer));
			
}

/*outputToExt opens a file for appending, adding a reference to an external symbol. It prints the symbol name followed by its address.*/
void outputToExt(int address, char *name, char *fileName)
{
	FILE *extFP; /*pointer to the .ent file.*/
	char buffer[MAXDIGITS], extFileName[MAXFILENAME]; /*buffers to hold the reference address and .ext full file name respectively.*/

	extFP = fopen(addFileEnd(extFileName,fileName, ".ext"), "a");

	fprintf(extFP, "%s\t%s\n", name, convertTo32Base(address, buffer));

	fclose(extFP);
}

/*convertTo32Base converts a decimal integer into a weird 32 base character, stores into a given buffer and also returns reference to the buffer. This function utilizes a mask to copy just half of the decNum bits at a time.*/
char *convertTo32Base(int decNum, char *buffer)
{
	int mask = (1 << 5) - 1; /*the mask 0000011111, to cover exactly half of a 10 bit line of code.*/
	char base32[] = {'!', '@', '#', '$', '%', '^', '&', '*', '<', '>', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v'}; /*An array where the position of each char represents its 32 base decimal value. */

	buffer[2] = '\0';
	
	buffer[1] = base32[decNum & mask];

	mask = mask << SECOND_HALF_BITS;

	buffer[0] = base32[((decNum & mask) >> SECOND_HALF_BITS)];

	return buffer;
}

/*isSavedWord compares a given string with all saved words stored in an array savedNames[]. returns an enum LINE_TYPES (in assembler.h) of saved word if match found, else returns the value NUMOFDIRS + NUMOFCMDS.*/
int isSavedWord(char *str)
{
	int j;
	char *savedNames[] = {"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop", "entry", "extern", "data", "string", "struct"}; /*An array where the position of each saved name represents its enum LINE_TYPES value.*/

	for(j = 0; (j < NUMOFDIRS + NUMOFCMDS) ; j++)
		if(!strcmp(str, savedNames[j]))
			return j;

	return j;
}

/*addFileEnd concatenates ending to the end of origin, storing result in dest. A reference to dest is returned.*/
char *addFileEnd(char *dest, char *origin, char *ending)
{
	strcpy(dest, origin);
	return strcat(dest, ending);
}

/*updateTablesAddress updates the address of data lines in the data and symbol tables, by adding IC, the number of command lines.*/
void updateTablesAddress(symboltable symTable, table dataTable, table cmdTable, int IC, int DC, int SC)
{
	int i;
	
	updateSymbolTableData(IC, SC, symTable);

	for(i = 0; i < IC; i++)
		cmdTable[i].address += MEMORYSTART;

	for(i = 0; i < DC; i++)
		dataTable[i].address += (IC + MEMORYSTART);
}

/*numOfOperands receives an enum LINE_TYPES and returns how many operands this command requires.*/
int numOfOperands(int cmdNum)
{
	switch(cmdNum)
	{
		case MOV:
		case CMP:
		case ADD:
		case SUB:
		case LEA:
			return 2;
			break;
		case NOT:
		case CLR:
		case INC:
		case DEC:
		case JMP:
		case BNE:
		case RED:
		case PRN:
		case JSR:
			return 1;
			break;
		case RTS:
		case STOP:
		default:
			return 0;
			break;
	}
}

/*getOperandIndex parses lineBuff char by char, skips the label (if declared, monitored by hasLabel) and command/directive. Stores the location of first and second operands in firstIndex and secondIndex respectively. If no operands were found, both indexes will be 0. If only 1 operand found, firstIndex = 0 and secondIndex will be the operand position.
If function caller gives secondIndex parameter as NULL, the function will only store first operand encountered, if any.*/
void getOperandIndex(int *firstIndex, int *secondIndex, char *lineBuff)
{
	int i = 0, hasLabel = 0; /*hasLabel is used to monitor if a label declaration was encountered.*/

	/*skip first word*/
	skip_space(lineBuff, i);
	for(; !isspace(lineBuff[i]); i++)
		if(lineBuff[i] == ':')
			hasLabel++;
	
	skip_space(lineBuff, i);

	/*if ':' char was encountered, label is there and it is needed to skip another word.*/
	if(hasLabel) 
	{
		skip_non_space(lineBuff, i);
		skip_space(lineBuff, i);
	}
		
	*firstIndex = i;

	for(; !isspace(lineBuff[i]) && (lineBuff[i] != '\0') && (lineBuff[i] != ','); i++)
		;

	for(; (lineBuff[i] == ',') || isspace(lineBuff[i]); i++)
		;

	if(*firstIndex == i) /*no operands*/
		*firstIndex = 0;

	if(secondIndex != NULL) /*means function caller wants second index as well, if present.*/ 
	{
		if(!lineBuff[i]) /*only 1 operand*/
		{
			*secondIndex = *firstIndex;
			*firstIndex = 0;
		}
		else /*2 operands*/
			*secondIndex = i;
	}
	
}

/*secondOperandIndex skips whole line until second operand given in lineBuff, returns position.*/
int secondOperandIndex(char *lineBuff)
{
	int i;

	for(i = 0; lineBuff[i] != ','; i++)
		;

	for(i = 0; isspace(lineBuff[i]); i++)
		;

	return i;
}


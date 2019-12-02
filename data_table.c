#include "assembler.h"
#include "utils.h"
#include <stdlib.h>
#include <ctype.h>

#define STRUCTOPERANDS 2


enum DATA_STATES {BEFORE_NUM, IN_NUM, OUT_NUM};

int getDataOperands(char *, int *, int );
int getStringOperands(char *, char *, int );
int getStructOperands(char *, int *, char *);

/*insertDataTable reads integer operands from a .data directive line and stores them in data table. Returns VALID if no errors, else returns error id.*/
int insertDataTable(char *lineBuff, int *DC, table dataTable)
{
	int numOfMemorySlots = 0, i; /*numOfMemorySlots holds amount of lines needed in data table for operands read from text.*/
	int operands[MAXLINE]; /*An array to hold the integer operands.*/

	numOfMemorySlots = getDataOperands(lineBuff, operands, MAXLINE);

	if(!numOfMemorySlots) /*must be at least 1 integer operand*/
		return ERR_DATA_OPERANDS;
	
	for(i = 0; i < numOfMemorySlots; i++) 
	{
		dataTable[*DC + i].address = *DC + i;
		dataTable[*DC + i].code = operands[i];
	}

	*DC += numOfMemorySlots;

	return VALID;
}

/*insertStringTable reads a string of chars from a .string directive line, stores them in data table and updates DC. Returns VALID if no errors, else returns error id.*/
int insertStringTable(char *lineBuff, int *DC, table dataTable)
{
	int numOfMemorySlots = 0, i; /*same as in insertDataTable*/
	int index; /*Will hold index of first char after left '"'. */
	char operands[MAXLINE]; /*Will hold the string of chars with ending '\0'.*/

	getOperandIndex(&index, NULL, lineBuff);
	numOfMemorySlots = getStringOperands(lineBuff, operands, index);

	
	if(!numOfMemorySlots) /*will be 0 if illegal string, even empty string takes 1 memory slot for ending '\0' char.*/
		return ERR_STRING_OPERANDS;

	for(i = 0; i < numOfMemorySlots; i++) /*includes '\0' char.*/
	{
		dataTable[*DC + i].address = *DC + i;
		dataTable[*DC + i].code = operands[i];
	}

	*DC += numOfMemorySlots;

	return VALID;
}

/*insertStructTable reads an integer operand followed by a string in a .struct directive line, inserts address and value  into data table and updatesDC. Returns VALID if no errors, else returns error id.*/
int insertStructTable(char *lineBuff, int *DC, table dataTable)
{
	int i = 0, numOperand;
	char strOperand[MAXLINE];
	
	if(getStructOperands(lineBuff, &numOperand, strOperand) < STRUCTOPERANDS) /*a struct must have 2 fields, int and a string.*/
		return ERR_STRUCT_OPERANDS;

	/*insert integer operand to data table*/
	dataTable[*DC].address = *DC;
	dataTable[*DC].code = numOperand;
	*DC += 1;

	/*insert string operand to data table*/
	do 
	{
		dataTable[*DC + i].address = *DC + i;
		dataTable[*DC + i].code = strOperand[i];
	}while(strOperand[i++]);
	
	*DC += i;

	return VALID;
}

/*getDataOperands reads int operands from lineBuff, stores them into operands and returns number of integers successfully read. numOfOperands is the max number of data operands to recieve, useful for struct that needs only 1 int.*/
int getDataOperands(char *lineBuff, int *operands, int numOfOperands)
{
	int index, j = 0, i = 0;
	int state = BEFORE_NUM; /*state will be of enum DATA_STATES type, will show state of loop while reading sentence.*/
	char c, numBuff[MAXNUM]; /*numBuff will hold the string representing the integer operand.*/

	getOperandIndex(&index, NULL, lineBuff);
	/*index = firstOperandIndex(lineBuff);*/
	
	for(; (c = lineBuff[index]) && (numOfOperands); index++)
	{
		switch(state)
		{
			case BEFORE_NUM:
				if(c == '+' || c == '-' || isdigit(c)) /* c is part of number*/
				{	
					numBuff[j++] = c;
					state = IN_NUM;
				}
				else if(!isspace(c))
						return 0; /*illegal character, 0 operands recieved.*/
				break;
			case IN_NUM: 
				if(isdigit(c)) 
					numBuff[j++] = c;
				else
					if((isspace(c) || c == ',') && isdigit(numBuff[j - 1])) /*space or comma ends the integer, cases of only "+" or "-" are not legal integers.*/
					{
						numBuff[j] = '\0';
						operands[i++] = atof(numBuff);
						j = 0;
						numBuff[0] = '\0';
						numOfOperands--;
						state = OUT_NUM;
					}
					else 
						return 0;

				if(c == ',') /*a comma is mandatory and checked in case: OUT_NUM.*/
					index--;
				break;
			case OUT_NUM:
				if(c == ',')
					state = BEFORE_NUM;
				else if(!isspace(c)) /*must be comma between operands*/
					return 0;
				break;
			default: break;
		}
	}

	return i;
}

/*getStringOperands reads a string operand from a .string directive line type, copies string into operands with ending '\0', returns num of chars read (including ending '\0').*/
int getStringOperands(char *lineBuff, char *operands, int start)
{
	int i, j = 0;

	for(i = start; (lineBuff[i] != '"') && (lineBuff[i]) ; i++) /*Searches for left '"' character.*/
		;

	if(!lineBuff[i]) /*if none found, no string is defined.*/
		return 0;

	for(i += 1 ; (lineBuff[i] != '"') && (lineBuff[i]); i++) /*reads chars into operands until right '"' found.*/
		operands[j++] = lineBuff[i];

	if(!lineBuff[i]) /*if none found, string is illegal.*/
		return 0;

	operands[j++] = '\0';
	
	return j;
}

/*getStructOperands utilizes getDataOperands() and getStringOperands() to read 1 integer and 1 string operands.*/
int getStructOperands(char *lineBuff, int *numOperand, char *strOperand)
{
	int numRecieved = 0;

	numRecieved += getDataOperands(lineBuff, numOperand, 1);

	if(numRecieved)
		numRecieved += (getStringOperands(lineBuff, strOperand, secondOperandIndex(lineBuff)) ? 1 : 0); 

	if(numRecieved >= 2)
		return numRecieved;
	else
		return INVALID;
}




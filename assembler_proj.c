/*
assembler_proj:
This assembler reads files of type .as, translates them into "weird 32 base" numbers and creates 3 output files for each:
.ob file- contains the commands and data. 
.ext file- a list of lines containing an external symbol.
.ent- entry symbols to be exported.
The names of the files to be translated are recieved as input in the command line argument.
The main function is an implementation of the two-scan algorithm, as described in the project description.

The modules used are as following:
cmd_table- handles inserting command and operands data into the command table, from command lines.
data_table- handles inserting integers, strings and structs into the data table, from directive lines.
symbol_table- handles insertion and queries of symbols in the symbol table.
first_scan- determines type of line (command/directive/empty/comment).
utils- contains utility functions such as text reading, translation to 32 base, creating output files and some text parsing functions.

This program assumes a maximum number of lines, label size and line length in each text file (as described in MAXTABLE, MAXLABEL and MAXLINE respectively in "assembler.h").
Also the program assumes memory starts at MEMORYSTART.
*/
#include <stdio.h>
#include "assembler.h"
#include "tables.h"
#include "utils.h"
#include "line_scan.h"

void handleError(int , int , char *);

/*main function receives file name (without the.as ending) as command line arguments, outputting neccessary files and handling errors found.*/
int main(int argc, char *argv[])
{
	FILE *fp;
	int i, STATUS, isErr = 0;

	for(i = 1; i < argc; i++)
	{
		table dataTable, cmdTable; /*tables to store data and command code lines, and their address.*/
		symboltable symTable; /*A symbol table used for storing declared labels and info about them.*/
		int DC = 0, IC = 0, SC = 0; /*Counters for the tables mentioned above. (DC-data, IC-cmd, SC-symbol)*/
		int lineCounter = 1; /*lineCounter counts each line of text encountered in source file, used mainly for error reference.*/
		int lineType; /*lineType will store the type of each line, as enum LINE_TYPES.*/
		char lineBuff[MAXLINE]; /*Buffer for current line being read.*/
		char fileName[MAXFILENAME], extFileName[MAXFILENAME]; /*.ext and .as file name buffer, including endings*/
		char extNameBuff[MAXLABEL]; /*Buffer for name of any external symbol encountered*/
		extline extSymbol; /*a typedef for holding a reference to an eternal symbol*/

		extSymbol.name = extNameBuff;

		addFileEnd(fileName, argv[i], ".as");
		
		if(!(fp = fopen(fileName, "r"))) /*Open next file, handle error if fopen fails.*/
		{
			fprintf(stderr, "\nerror: the assembler was unable to open the file %s\n", fileName);
			continue;
		}
		
		/*First Scan*/

		while(fgets(lineBuff, MAXLINE, fp))/*read next line of text into lineBuff, until EOF*/ 
		{	
			char label[MAXLABEL]; /*Will hold name of label, also works as label_flag (No label if label[0]=='\0').*/
			STATUS = VALID;

			if((STATUS = getLineTypeScan(lineBuff, &lineType, label)) == VALID) /*determine type of line by reading first word (and label, if one exists)*/
			{
				switch(lineType)
				{	
					case DATA:
						if(label[0] != '\0')
							insertDataSymbol(label, DC, SC++, symTable); 
						STATUS = insertDataTable(lineBuff, &DC, dataTable);/*DC will now be updated by amount of memory words added.*/
						break;
					case STR:
						if(label[0] != '\0')
							insertDataSymbol(label, DC, SC++, symTable); 
						STATUS = insertStringTable(lineBuff, &DC, dataTable);
						break;
					case STRUCT:
						if(label[0] != '\0')
							insertDataSymbol(label, DC, SC++, symTable); 
						STATUS = insertStructTable(lineBuff, &DC, dataTable);
						break;
					case EXT:
						insertExternSymbol(lineBuff, SC++, symTable);
						break;
					case ENT: /*skip line*/
						break; 
					default: /*must be command*/
						if(label[0] != '\0')
							insertCmdSymbol(label, IC, SC++, symTable);
						STATUS = insertCmdTable(lineBuff, &IC, lineType, cmdTable);/*IC will be updated by amount of memory words needed.*/
						break;
				}
			}
			handleError(STATUS, lineCounter++, argv[i]); /*will print to stderr if necessary*/
			if(STATUS > EMPTY_LINE)
				isErr++;
		} 
		
		if(isErr)
		{
			fclose(fp);
			continue; /*No need for second scan if errors found.*/
		}

		updateTablesAddress( symTable, dataTable, cmdTable, IC, DC, SC); /*updates the address of the data/cmd/symbol tables..*/

		/*Second Scan*/
		rewind(fp);
		IC = 0, lineCounter = 1;

		while(fgets(lineBuff, MAXLINE, fp))
		{
			STATUS = VALID;
			if((STATUS = getLineTypeScan(lineBuff, &lineType, NULL)) == VALID)
			{
				if(lineType == ENT)
					STATUS = updateEntrySymbol(lineBuff, symTable, SC);
				else if(lineType >= MOV && lineType <= STOP)
					STATUS = completeCmdTable(lineBuff, &IC, lineType, cmdTable, symTable, SC, argv[i], extSymbol); /*adds missing code words of operands, will also update IC*/
			}
			 
			if(STATUS > EMPTY_LINE) 
				isErr++;
			
			handleError(STATUS, lineCounter++, fileName);
		}

		if(!isErr){ /*No need for output if errors found.*/
			if(!createOutput(argv[i], IC, DC, SC, symTable, dataTable, cmdTable)) /*includes conversion to weird 32 base numbers.*/
				handleError(ERR_FAILED_FILE_OPEN, 0, fileName); 
		}
		else
			remove(addFileEnd(extFileName, argv[i], ".ext")); /*.ext file is created during first scan, so it must deleted only after errors are found.*/

		fclose(fp);
	}
	return 0;
}

/*handleError prints various responses to stderr by enum ERR_STATUS integer sent through errNum.*/
void handleError(int errNum, int lineNum, char *fileName)
{
	if(EMPTY_LINE < errNum)
	{
		fprintf(stderr, "Error in file %s, line %d: \n", fileName, lineNum); /*general error reference format, line number in source text.*/
		switch(errNum)
		{
			case ERR_NEW_LINE:
				fprintf(stderr, "\tLines must be seperated by newline character. \n");
				break;
			case ERR_FIRST_NOT_ALPHA:
				fprintf(stderr, "\tA label must begin with upper or lower case letter. \n");
				break;
			case ERR_NOT_ALNUM:
				fprintf(stderr, "\tA label must contain only  alphabetic and numeric characters. \n");
				break;
			case ERR_UNKNOWN_CMD:
				fprintf(stderr, "\tUnknown command. Command must be one of the 16 known commands. \n");
				break;
			case ERR_UNKNOWN_DIR:
				fprintf(stderr, "\tUnknown directive. Must be one of 5 known directives (.extern/.entry/.data/.string/.struct).\n");
				break;
			case ERR_LABEL_SAVED_WORD:
				fprintf(stderr, "\tTried to define label name as saved word. \n");
				break;
			case ERR_NO_ENTRY_EXISTS:
				fprintf(stderr, "\tTried to define unknown symbol as entry. \n");
				break;
			case ERR_DATA_OPERANDS:
				fprintf(stderr, "\t.data directive may only be followed by integer values, at least one or more, seperated by commas. \n");
				break;
			case ERR_STRING_OPERANDS:
				fprintf(stderr, "\t.string directive must be followed by two '\"' characters, with the desired string between them. \n");
				break;
			case ERR_STRUCT_OPERANDS:
				fprintf(stderr, "\t.struct directive must be followed by one integer and one string, seperated by comma. \n");
				break;
			case ERR_WRONG_NUM_OPS:
				fprintf(stderr, "\tNumber of input parameters must match required parameters of command. \n");
				break;
			case ERR_SYMBOL_NOT_FOUND:
				fprintf(stderr, "\tCould not find given parameter label in symbol table. \n");
				break;
			case ERR_FAILED_FILE_OPEN:
				fprintf(stderr, "\tAssembler failed to create output files. \n");
				break;
			case ERR_FAILED_FILE_REMOVE:
				fprintf(stderr, "\tAssembler failed to remove unneeded %s.ext file. \n", fileName);
				break;
			case ERR_UNKNOWN_REGISTER:
				fprintf(stderr, "\tA register must be one of 8 available registers, r0 through r7. \n");
				break;
			default:
				break;
		}
	}
}





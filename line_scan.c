#include <ctype.h>
#include <string.h>
#include "assembler.h"
#include "tables.h"
#include "utils.h"

#define IS_NOT_LABEL(name) ((name) ? name[0] == '\0' : TRUE)

/*Various states of parsing in getLineTypeScan() function. Starts from 100 to distinguish from enum LINE_TYPES.*/
enum SENTENCE_STATE {QUIT = 100, OUT, IN_DIRECTIVE, IN_COMMAND, OUT_COMMAND, OUT_LABEL};

int caseOut(char, char *);
int caseInCommand(char, char *);
int caseOutCommand(char *, int *);
int caseInDirective(char *, int, char *, int *);

/*getLineTypeScan function receives a source line as string, determines type of sentence and whether or not a label is declared.
 It reads line char by char, each time determining current state (enum SENTENCE_STATE) and responds accordingly. returns an enum ERR_STATUS indicator.*/
int getLineTypeScan(char *line, int *type, char *label)
{
	int i, state = OUT, length = 0; /*state holds current state in line, length is a char counter for each word.*/
	char wordBuffer[MAXLABEL]; /*buffer for words read.*/

	wordBuffer[0] = '\0';
	
	if(label != NULL) 
		label[0] = '\0';

	for(i = 0; (line[i] != '\0') || (state == OUT_COMMAND); i++)
	{
		switch(state)
		{
			case OUT: 
				state = caseOut(line[i], label);
				break;
			case IN_COMMAND:
				wordBuffer[length++] = line[i - 1];
				state = caseInCommand(line[i], label);
				break;
			case OUT_COMMAND:
				wordBuffer[length] = '\0';
				return caseOutCommand(wordBuffer, type);
				break;
			case IN_DIRECTIVE:
				return caseInDirective(line, i, wordBuffer, type);
				break;
			case OUT_LABEL:
				if(label) /*if label == NULL, caller does not need to save label*/
				{
					wordBuffer[length] = '\0';
					strcpy(label, wordBuffer);
					if(isSavedWord(label) < NUMOFCMDS + NUMOFDIRS)
						return ERR_LABEL_SAVED_WORD;
				}
				length = 0;
				wordBuffer[0] = '\0';
				state = caseOut(line[i], label);
				break;
			default: /*if none of the above, must be an error code from enum ERR_STATUS*/
				return state;
		}
	}
	
	if(line[i] == '\0' && line[i-1] != '\n') /*line must end with newline character*/	
		return ERR_NEW_LINE; 
	else if(wordBuffer[0] == '\0') 
		return EMPTY_LINE; 
	else
		return VALID;
}

/*caseOut returns state to assume or error if any is detected.*/
int caseOut(char c, char *label)
{
	if(!isspace(c))
	{
		if(c == '\0')
			return EMPTY_LINE;
		else if(c == ';')
			return COMMENT_LINE;
		else if(c == '.')
			return IN_DIRECTIVE;
		else if(!isalpha(c) && ((label) ? label[0] == '\0' : 0)) /*label must begin with upper or lower case letter, in case label is still undefined.*/
			return ERR_FIRST_NOT_ALPHA;
		else /*isalpha == true*/
			return IN_COMMAND;
	}
	
	return OUT;
}

/*caseInCommand returns state to assume or error if any is detected.*/
int caseInCommand(char c, char *label)
{
	if(isspace(c) || !c)
		return OUT_COMMAND;
	else if(((label) ? (label[0] != '\0') : 0)) /*if label is defined already, this must be cmd*/
		return IN_COMMAND;
	else if(c == ':') 
		return OUT_LABEL;
	else if(!isalnum(c)) /*labels can be numbers and letters, cmd only letters*/
		return ERR_NOT_ALNUM;

	return IN_COMMAND;
}

/*caseOutCommand returns VALID if proper command, else error..*/
int caseOutCommand(char *wordBuffer, int *type)
{
	int j;

	j = isSavedWord(wordBuffer);

	if(j >= NUMOFCMDS) 
		return ERR_UNKNOWN_CMD;
	else 
	{
		*type = j;
		return VALID;
	}
}

/*caseInDirective returns valid if proper directive, else returns error.*/
int caseInDirective(char *line, int i, char *wordBuffer, int *type)
{
	int j;

	for(j = 0; (islower(line[j + i])); j++)
		wordBuffer[j] = line[j + i];
	
	wordBuffer[j] = '\0';

	j = isSavedWord(wordBuffer);

	if((j == NUMOFDIRS + NUMOFCMDS) || (j < NUMOFCMDS))
		return ERR_UNKNOWN_DIR;
	else
	{
		*type = j;
		return VALID;
	}
}












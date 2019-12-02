/*Receives a text line to read and store type of line at given address (as enum LINE_TYPES). returns VALID if successful, else returns error.
line- Line to read. type- address to store enum LINE_TYPES integer. label- buffer to store label, if one is declared.
label may be NULL if any label declaration is to be ignored.*/
int getLineTypeScan(char *line, int *type, char *label);

assembler_proj: assembler_proj.o line_scan.o utils.o cmd_table.o data_table.o symbol_table.o
	gcc -ggdb -g -Wall -ansi -pedantic assembler_proj.o line_scan.o utils.o cmd_table.o data_table.o symbol_table.o -o assembler_proj

assembler_proj.o: assembler_proj.c assembler.h tables.h utils.h line_scan.h
	gcc -ggdb -c -Wall -ansi -pedantic assembler_proj.c -o assembler_proj.o

line_scan.o: line_scan.c assembler.h utils.h tables.h
	gcc -ggdb -c -Wall -ansi -pedantic line_scan.c -o line_scan.o

utils.o: utils.c assembler.h tables.h utils.h
	gcc -ggdb -c -Wall -ansi -pedantic utils.c -o utils.o

cmd_table.o: cmd_table.c tables.h assembler.h utils.h
	gcc -ggdb -c -Wall -ansi -pedantic cmd_table.c -o cmd_table.o

data_table.o: data_table.c tables.h assembler.h 
	gcc -ggdb -c -Wall -ansi -pedantic data_table.c -o data_table.o

symbol_table.o: symbol_table.c tables.h assembler.h utils.h
	gcc -ggdb -c -Wall -ansi -pedantic symbol_table.c -o symbol_table.o

clean:
	rm *~ assembler_proj *o

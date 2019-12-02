;An example of all directives, some types of command and all command operands.


MAIN: add NUM1 , NUM2

  .extern NUM1
  .extern NUM2
  .entry LBL

mov #3 , #4
sub r2, r4
dec IND
IND:	.data -45 , 00054, +23
STRING: .string "abcdef"  
red STRING.1
red STRING.2
LBL: .struct 33 , "struct_example"   


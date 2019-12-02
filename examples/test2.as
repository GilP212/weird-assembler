;This is the example given in Course Book.

MAIN:		mov S1.1 ,LENGTH
		add r2,STR
LOOP:		jmp END
		prn #-5
		sub r1, r4
		inc K

		mov S1.2 ,r3
		bne LOOP
END: 		stop
STR:		.string "abcdef"
LENGTH:		.data 6,-9,15
K:		.data 22
S1:		.struct 8, "ab"

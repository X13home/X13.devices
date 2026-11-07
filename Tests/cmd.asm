	NOP
	LD	0
	LD	1
	LD	-1
	LD	255
	LD	-127
	LD	256
	LD	-128
	LD	65536
	LD P0
	LD L0
      	LD z0
      	LD b0
      	LD B0
      	LD w0
      	LD W0
      	LD d0
      	ST P0
      	ST L0
      	ST z0
      	ST b0
      	ST B0
      	ST w0
      	ST W0
      	ST d0

	DUP
	DROP
	NIP
	SWAP
	OVER
	ROT
	NOT
	AND
	OR
	XOR
	SHL
	SHR
	INC
	DEC
	NEG
	ADD
	SUB
	MUL
	DIV
	MOD
	CEQ
	CNE
	NOT_L
	AND_L
	OR_L
	XOR_L
	CGT
	CGE
	CLT
	CLE

      	JMP 	L0
:L0    	JZ 	L1
:L1    	JNZ 	L2
:L2    	CALL 	L3
:L3    	RET 

	TEST_EQ 0

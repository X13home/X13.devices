	NOP
	NOP
	NOP
    NOP

	LD	81
	LD	82
	CEQ
	TEST_EQ	0
    
    NOP
    
	LD	83
	DUP
	CEQ
	TEST_EQ	1
    
    NOP

	LD	84
	LD	85
	CNE
	TEST_EQ	1
    
    NOP

	LD	86
	DUP
	CNE
	TEST_EQ	0
    
    RET

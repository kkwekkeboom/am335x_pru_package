.origin 0	
.entrypoint START

#include "PRU_spidriver.hp"

#define GPIO30_15 (1<<15) 

START:
	 // Enable OCP master port
	LBCO 	r0, CONST_PRUCFG, SYSCFG_OFFSET, 4		//load SYSCF_reg
    CLR  	r0, r0, 4 								//clear STANDBY_INIT
    SBCO 	r0, CONST_PRUCFG, SYSCFG_OFFSET, 4		//store SYSCF_reg

  	// Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
    // c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
    // to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).    
    MOV     r0, 0x00000000
    MOV     r1, CTBIR_0
    SBBO    r0, r1,0,4
    
    //Load 4 bytes from memory location c3(PRU0/1 Local Data)+4 into r4 using constant table
	LBCO    r0, CONST_PRUDRAM, 0, 4 
	
    //check if bit 15 of the shared memory is set
    MOV		r1, GPIO30_15
    QBBS 	SET_PIN, r0, r1
    
CLEAR_PIN:
    //set output pin according value in shared memory
    MOV 	r3, GPIO30_15
    NOT 	r3, r3
    AND 	r30, r30, r3
    //Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
    //MOV 	r3, 0x0000
    //SBCO    r3, CONST_PRUDRAM, 0, 4
    JMP 	END_SET_PIN
SET_PIN:
	MOV 	r3, GPIO30_15
	OR	 	r30, r30, r3
	//Store result in into memory location c3(PRU0/1 Local Data)+8 using constant table
    MOV 	r3, 0x00FF
    SBCO    r3, CONST_PRUDRAM, 0, 4
END_SET_PIN:
	//OR r30, r30, r2
	MOV	r31.b0, PRU0_ARM_INTERRUPT + 16
	HALT

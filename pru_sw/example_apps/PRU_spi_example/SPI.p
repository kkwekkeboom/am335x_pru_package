.origin 0
.entrypoint START
#include "SPI.hp"

START:
    // Enable OCP master port
    LBCO r0, CONST_PRUCFG, 4, 4
    CLR  r0, r0, 4
    SBCO r0, CONST_PRUCFG, 4, 4

    //C28 will point to 0x00012000 (PRU shared RAM)
    MOV  r0, 0x00000120
    MOV  r1, CTPPR_0
    ST32 r0, r1
  
    // Enable CLKSPIREF and CLK
    MOV  r1, 0x44E00050
    MOV  r2,  0x00000002
    SBBO r2,  r1, 0, 4
    
    // Reset SPI
    MOV  r1, 0x481A0110
    LBBO r2,  r1, 0, 4
    SET  r2.t1
    SBBO r2,  r1, 0, 4
    
    //Wait for RESET
    RESET:
  	MOV  r1,  0x481a0114
  	LBBO r2,   r1, 0, 4
  	QBBC RESET, r2.t0
    
    //Config MODULCTRL
    MOV  r1, 0x481A0128
    MOV  r2,  0x00000000
    SBBO r2,  r1 , 0, 4
    
    //Config SYSCONFIG
    MOV  r1, 0x481A0110
    MOV  r2,  0x00000311
    SBBO r2,  r1, 0, 4
    
    //Reset interrupt status bits
    MOV  r1, 0x481A0118
    MOV  r2,  0xFFFFFFFF
    SBBO r2,  r1, 0, 4
    
    //Disable interupts
    MOV  r1, 0x481A011C
    MOV  r2,  0x00000000
    SBBO r2,  r1, 0, 4
  
    // Disable channel 1
    MOV  r1, 0x481A0148
    MOV  r2,  0x00000000
    SBBO r2,  r1, 0, 4
    
    // Configure channel 1 of MCSPI1
    MOV  r1, 0x481A0140  
    MOV  r2, 0x000192FC0
    LBCO r4, CONST_PRUSHAREDRAM, 8, 4 //frequency
    OR   r2, r2,r4
    SBBO r2, r1, 0, 4

    LBCO r3, CONST_PRUSHAREDRAM, 4, 4 //num times to transmit

    MOV  r1, 0x481A014C
    LBCO r2, CONST_PRUSHAREDRAM, 0, 4 //word to transmit
    SBBO r2,  r1, 0, 4

// ******************** BEGIN ACQUISITION ********************
RUN_AQ:

// Write DAC command to SPI_TX1
CALL ENABLE_CH1
MOV  r1, 0x481A014C
LBCO r2, CONST_PRUSHAREDRAM, 0, 4 //word to transmit
SBBO r2,  r1, 0, 4

SUB r3, r3, 1
QBEQ EXIT, r3, 0
JMP RUN_AQ

//////////////////////////////////////
CHECKTX1:
MOV  r1, 0x481A0144
LBBO r2, r1, 0, 4
QBBC CHECKTX1, r2.t1
JMP  r18.w0

ENABLE_CH1:
// Enable Channel 1
MOV  r1, 0x481A0148
MOV  r2, 0x00000001
SBBO r2, r1, 0, 4
JAL  r19.w0, CHECKTX1
RET

EXIT:
  //Disable channel 1
  MOV  r1, 0x481A0148
  MOV  r2,  0x00000000
  SBBO r2,  r1, 0 ,4
  
  MOV R31.b0, PRU0_ARM_INTERRUPT+16
  HALT


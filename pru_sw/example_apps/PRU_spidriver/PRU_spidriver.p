.origin 0	
.entrypoint START

#include "PRU_spidriver.hp"

#define GPIO30_15 (1<<15) 

START:
	 // Enable OCP master port
	LBCO 	r0, CONST_PRUCFG, SYSCFG_OFFSET, 4		//load SYSCF_reg
    CLR  	r0, r0, 4 								//clear STANDBY_INIT
    SBCO 	r0, CONST_PRUCFG, SYSCFG_OFFSET, 4		//store SYSCF_reg
    
    // Set OCP address offset to -0x00080000 for PRU0
   // LBCO	r0, CONST_PRUCFG, PMAO, 4
   // SET		r0, r0, 1
   // SBCO	r0, CONST_PRUCFG, PMAO, 4  

  	// Configure the block index register for PRU0 by setting c24_blk_index[7:0] and
    // c25_blk_index[7:0] field to 0x00 and 0x00, respectively.  This will make C24 point
    // to 0x00000000 (PRU0 DRAM) and C25 point to 0x00002000 (PRU1 DRAM).    
    MOV     r0, 0x00000000
    MOV     r1, CTBIR_0
    SBBO    r0, r1,0,4
    
SPI_PROGRAM:
 	// Enable CLKSPIREF and CLK
    MOV  	r1, SPI1_CLOCK
    MOV  	r2,  0x00000002
    SBBO 	r2,  r1, 0, 4
    
    // Reset SPI
    MOV 	r1, SPI1_BASE
    MOV		r2, MCSPI_SYSCONFIG
    LBBO 	r3, r1, r2, 4
    SET  	r3.t1
    SBBO 	r3, r1, r2, 4
    
    //Wait for RESET
    RESET:
    MOV		r2, MCSPI_SYSSTATUS
  	LBBO 	r3, r1, r2, 4
  	QBBC 	RESET, r3.t0
  	
    // Wake up device
    //MOV 	r2, MCSPI_WAKEUPENABLE
    //LBBO	r3, r1, r2, 4
    //OR		r3, r3, (1<<WKEN)
    //SBBO	r3, r1, r2, 4
  	
  	//Disable all channels (just in case)
  	//MOV		r2, MCSPI_CH0CTRL
  	//MOV  	r3, 0x00000000
	//SBBO 	r3, r1, r2, 4
	//MOV		r2, MCSPI_CH1CTRL
	//SBBO 	r3, r1, r2, 4
	//MOV		r2, MCSPI_CH2CTRL
	//SBBO 	r3, r1, r2, 4
	//MOV		r2, MCSPI_CH3CTRL
	//SBBO 	r3, r1, r2, 4
  	
  	//Config SYS: set SSB before clearing IRQSTATUS reg
  	MOV		r2, MCSPI_SYST
  	LBBO	r3, r1, r2, 4
  	MOV		r4, (1 << SPIDATDIR0 | 1 << SSB)
  	OR		r3, r3, r4
  	MOV		r4, (~(1 << SPIDATDIR1 | 1 << SPIENDIR))
  	SBBO	r3, r1, r2, 4
    
    //Config MODULCTRL: No select lines used, single channel mode
    MOV		r2, MCSPI_MODULECTRL
    LBBO	r3, r1, r2, 4
    MOV		r4, (~(1 << MS | 1 << SYSTEST))
    AND		r3, r3, r4
    OR  	r3, r3, (1<< SINGLE_CHANNEL)
    SBBO 	r3, r1, r2, 4
    
    //Config SYSCONFIG:
    MOV		r2, MCSPI_SYSCONFIG
    MOV  	r3, (3 << CLOCK_ACTIVITY| 1 << SIDLEMODE| 1 << AUTOIDLE)
    SBBO 	r3, r1, r2, 4
    
    //Reset interrupt status bits
    MOV		r2, MCSPI_IRQSTATUS
    MOV  	r3,  0xFFFFFFFF
    SBBO 	r3, r1, r2, 4
    
    //Disable interupts
    MOV		r2, MCSPI_IRQENABLE
    MOV  	r3,  0x00000000
    SBBO 	r3, r1, r2, 4
    
// 	Do actual tranfer
ENABLE_CH1:
	//Set configuration of channel 0 //1 << FFER |1 << FFEW|
    MOV		r2, MCSPI_CH0CONF 
    MOV  	r3, (1 << DEP0 | 23 << WL | 10 << CLKD | SPI_MODE_1)
    MOV		r4, (~(1 << IS | 1 << DEP1 | 1 << TRM0 | 1 << TRM1)) 
    AND		r3, r3, r4
    SBBO 	r3, r1, r2, 4
	
	// Set Channel0
	MOV		r2, MCSPI_CH0CTRL
	LBBO	r3, r1, r2, 4
	MOV		r4, (~(1<<EXTCLK))
	AND  	r3, r3, r4
	MOV		r4, (1 << EN)
	OR 		r3, r3, r4
	SBBO 	r3, r1, r2, 4
	
	// ENABLE SPIEN
	MOV		r2, MCSPI_CH0CONF
    LBBO	r3, r1, r2, 4
    MOV		r4, (1<<FORCE)
    OR		r3, r3, r4
    SBBO	r3, r1, r2, 4
    
    // Wait till TX register is empty
CHECKTX_SET:
	MOV 	r2, MCSPI_CH0STAT
	LBBO 	r3, r1, r2, 4
	QBBS 	CHECKTX_SET, r3.t1
	
	// Empty RX register
	MOV 	r2, MCSPI_RX0
	LBBO	r3, r1, r2, 4
		
	// Wait till RX register is empty
CHECKRX_CLR:
	MOV 	r2, MCSPI_CH0STAT
	LBBO 	r3, r1, r2, 4
	QBBS 	CHECKRX_CLR, r3.t0
	
	// Load data
	MOV		r2, MCSPI_TX0
    MOV	 	r3, 0x00A30000
	SBBO 	r3, r1, r2, 4

CHECK_EOT:
	MOV 	r2, MCSPI_CH0STAT
	LBBO 	r3, r1, r2, 4
	QBBS 	CHECK_EOT, r3.t2	

	// Wait till RX is not empty
CHECKRX_CLEAR0:
	MOV 	r2, MCSPI_CH0STAT
	LBBO 	r3, r1, r2, 4
	QBBC 	CHECKRX_CLEAR0, r3.t0
	
	// Ready reply
	MOV 	r2, MCSPI_RX0
	ADD		r2, r2, 3
	LBBO	r3, r1, r2, 1
	SBCO	r3, CONST_PRUDRAM, 4, 1
	
	SUB		r2, r2, 1
	LBBO	r3, r1, r2, 1
	SBCO	r3, CONST_PRUDRAM, 8, 1
	
	SUB		r2, r2, 1
	LBBO	r3, r1, r2, 1
	SBCO	r3, CONST_PRUDRAM, 12, 1
	
	SUB		r2, r2, 1
	LBBO	r3, r1, r2, 1
	SBCO	r3, CONST_PRUDRAM, 16, 1
	
EXIT:
  	//Disable channel 1
  	MOV		r2, MCSPI_CH0CTRL
  	MOV  	r3, 0x00000000
  	SBBO 	r3, r1, r2, 4 
	
	JMP END_SET_PIN
SET_PIN_PROGRAM:	
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
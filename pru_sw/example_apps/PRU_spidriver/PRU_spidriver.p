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
    MOV 	SPI1_OFFSET, SPI1_BASE
    MOV		r2, MCSPI_SYSCONFIG
    LBBO 	r3, SPI1_OFFSET, r2, 4
    SET  	r3.t1
    SBBO 	r3, SPI1_OFFSET, r2, 4
    
    //Wait for RESET
    RESET:
    MOV		r2, MCSPI_SYSSTATUS
  	LBBO 	r3, SPI1_OFFSET, r2, 4
  	QBBC 	RESET, r3.t0
  	
  	//Config SYS: set SSB before clearing IRQSTATUS reg
  	MOV		r2, MCSPI_SYST
  	LBBO	r3, SPI1_OFFSET, r2, 4
  	MOV		r4, (1 << SPIDATDIR0 | 1 << SSB)
  	OR		r3, r3, r4
  	SBBO	r3, SPI1_OFFSET, r2, 4
    
    //Config MODULCTRL: No select lines used, single channel mode
    MOV		r2, MCSPI_MODULECTRL
    LBBO	r3, SPI1_OFFSET, r2, 4
    MOV		r4, (~(1 << MS | 1 << SYSTEST))
    AND		r3, r3, r4
    OR  	r3, r3, (1<< SINGLE_CHANNEL)
    SBBO 	r3, SPI1_OFFSET, r2, 4
    
    //Config SYSCONFIG:
    MOV		r2, MCSPI_SYSCONFIG
    MOV  	r3, (3 << CLOCK_ACTIVITY| 1 << SIDLEMODE| 1 << AUTOIDLE)
    SBBO 	r3, SPI1_OFFSET, r2, 4
    
    //Reset interrupt status bits
    MOV		r2, MCSPI_IRQSTATUS
    MOV  	r3,  0xFFFFFFFF
    SBBO 	r3, SPI1_OFFSET, r2, 4
    
    //Disable interupts
    MOV		r2, MCSPI_IRQENABLE
    MOV  	r3,  0x00000000
    SBBO 	r3, SPI1_OFFSET, r2, 4
    
// 	Do actual tranfer
ENABLE_CH1:
	//Set configuration of channel 0 
    MOV		r2, MCSPI_CH0CONF 
    MOV  	r3, (1 << FFER | 1 << FFEW| 1 << DEP0 | 7 << WL | 10 << CLKD | SPI_MODE_1)
    MOV		r4, (~(1 << IS | 1 << DEP1 | 1 << TRM0 | 1 << TRM1)) 
    AND		r3, r3, r4
    SBBO 	r3, SPI1_OFFSET, r2, 4
	
	// Set Channel0
	MOV		r2, MCSPI_CH0CTRL
	LBBO	r3, SPI1_OFFSET, r2, 4
	MOV		r4, (~(1<<EXTCLK))
	AND  	r3, r3, r4
	MOV		r4, (1 << EN)
	OR 		r3, r3, r4
	SBBO 	r3, SPI1_OFFSET, r2, 4
	
	// ENABLE SPIEN
	MOV		r2, MCSPI_CH0CONF
    LBBO	r3, SPI1_OFFSET, r2, 4
    MOV		r4, (1<<FORCE)
    OR		r3, r3, r4
    SBBO	r3, SPI1_OFFSET, r2, 4
    
    // Wait till TX register is empty
CHECKTX_SET:
	MOV 	r2, MCSPI_CH0STAT
	LBBO 	r3, SPI1_OFFSET, r2, 4
	QBBS 	CHECKTX_SET, r3.t1
	
	// Wait till RX register is empty
CHECKRX_CLR:
	MOV 	r2, MCSPI_CH0STAT
	LBBO 	r3, SPI1_OFFSET, r2, 4
	QBBS 	CHECKRX_CLR, r3.t0
	
READ_MODULES:
	MOV		MODULE_COUNTER, 0
	MOV		MODULES_LENGTH, 16
	MOV		COMMAND_MEM_POINTER, 0 	//offset
	MOV		REPLY_MEM_OFFSET, 64 	//16 * 4 bytes offset

READ_MODULE:	
	LBCO	COMMAND, CONST_PRUDRAM, COMMAND_MEM_POINTER, 4
	call	SELECT_MODULE
	QBEQ	END_READ_MODULE, COMMAND, 0 //no command for this module
	
	MOV		BYTE_COUNTER, 0
	MOV		REPLY_MEM_POINTER, REPLY_MEM_OFFSET 
	MOV		MESSAGE_LENGTH, MAX_MESSAGE_LENGTH //max length SPI message
TRANSMIT:	
	QBEQ	TRANSMIT_STARTBYTE, BYTE_COUNTER, 0 //transmit startbyte or not
	ADD		r8, BYTE_COUNTER, 1 //last byte
	QBEQ	TRANSMIT_STOPBYTE, r8, MESSAGE_LENGTH 
	JMP		TRANSMIT_NORMALBYTE
TRANSMIT_STARTBYTE:
	// Load data
	MOV		r2, MCSPI_TX0
    MOV		r3, REQUEST_CMD
	SBBO 	r3, SPI1_OFFSET, r2, 4
	JMP		CHECK_EOT 
TRANSMIT_STOPBYTE:	
	MOV		r2, MCSPI_TX0
    MOV		r3, REQUEST_STOP
	SBBO 	r3, SPI1_OFFSET, r2, 4
	JMP		CHECK_EOT	
TRANSMIT_NORMALBYTE:	
	// Load data
	MOV		r2, MCSPI_TX0
    MOV		r3, 0x00000000
	SBBO 	r3, SPI1_OFFSET, r2, 4

CHECK_EOT:
	MOV 	r2, MCSPI_CH0STAT
	LBBO 	r3, SPI1_OFFSET, r2, 4
	QBBS 	CHECK_EOT, r3.t2

	// Wait till RX is not empty
CHECKRX_CLEAR:
	MOV 	r2, MCSPI_CH0STAT
	LBBO 	r3, SPI1_OFFSET, r2, 4
	QBBC 	CHECKRX_CLEAR, r3.t0
	
	// Ready reply
	MOV 	r2, MCSPI_RX0
	LBBO	r3, SPI1_OFFSET, r2, 1
	SBCO	r3, CONST_PRUDRAM, REPLY_MEM_POINTER, 1
	
	//Decode reply
	ADD		r8, BYTE_COUNTER, 1
	QBEQ	READ_STOPBYTE, r8, MESSAGE_LENGTH 	//last byte
	QBEQ	READ_LENGTH, BYTE_COUNTER, 1 //length
	QBEQ	READ_STARTBYTE, BYTE_COUNTER, 0 //start byte
	JMP		READ_NORMAL 
	
READ_STARTBYTE:
	MOV		r4, START_BYTE
	QBNE	WRONG_STARTBYTE, r3, r4 //if startbyte=0x23
	JMP		END_READ
WRONG_STARTBYTE:	
	ADD		MESSAGE_LENGTH, BYTE_COUNTER, 2	//Just send stopbyte
	JMP		END_READ
READ_LENGTH:
	MOV		r4, 0
	QBEQ	WRONG_LENGTH, r3, r4
	MOV		r4, 0xFF
	QBEQ	WRONG_LENGTH, r3, r4
	MOV		r4, 0xA1
	QBEQ	WRONG_LENGTH, r3, r4
	MOV		r4, INITIALIZING
	QBEQ	WRONG_LENGTH, r3, r4
	MOV		MESSAGE_LENGTH, r3	//set length to read length
	JMP END_READ
WRONG_LENGTH:
	ADD		MESSAGE_LENGTH, BYTE_COUNTER, 2 // Just send stopbyte
	JMP		END_READ
	
READ_NORMAL:
READ_STOPBYTE:
	
END_READ:	
	ADD		BYTE_COUNTER, BYTE_COUNTER, 1 //increment number of cycles
	ADD		REPLY_MEM_POINTER, REPLY_MEM_POINTER, 1 //increment RAM byte offset by one byte
	call 	DELAY_US
	QBLT	TRANSMIT, MESSAGE_LENGTH, BYTE_COUNTER
	
END_READ_MODULE:
	ADD		MODULE_COUNTER, MODULE_COUNTER, 1 //increment module counter
	MOV		r0, MAX_MESSAGE_LENGTH
	ADD		REPLY_MEM_OFFSET, REPLY_MEM_OFFSET, r0 //increment RAM offset by
	ADD		COMMAND_MEM_POINTER, COMMAND_MEM_POINTER, 4 //increment offset by 4
	QBLT	READ_MODULE, MODULES_LENGTH, MODULE_COUNTER 	
	
EXIT:
  	//Disable channel 1
  	MOV		r2, MCSPI_CH0CTRL
  	MOV  	r3, 0x00000000
  	SBBO 	r3, SPI1_OFFSET, r2, 4 
	MOV	r31.b0, PRU0_ARM_INTERRUPT + 16
	HALT
	
DELAY_US:
	MOV		r3, (1<<10)
DO_LOOP:	
	SUB		r3, r3, 1
	QBNE	DO_LOOP, r3, 0  
	RET
	
SELECT_MODULE: //assumes MODULE_COUNTER is set correctly
	//Simulate division
	MOV		r0, MODULE_COUNTER //numerator
	MOV		r1, 4 //denominator
	MOV		r2, 0 //quotient and backplanemask
	MOV		r3, 0 //modulus and modulemask
DIVIDE:	
	QBLT	END_DIVIDE, r1, r0 //stop if smaller than 4
	SUB		r0, r0, 4
	ADD		r2, r2, 1
	JMP		DIVIDE
END_DIVIDE:
	MOV 	r4, 1
	LSL		r2, r4, r2 //bit 0,1,2, or 3 will set
	LSL		r3, r4, r0 //bit 0,1,2, or 3 will set
	
	MOV		r4, GPIO0_BASE
	MOV		r5, GPIO1_BASE
	MOV		r6, GPIO2_BASE
	MOV		r7, GPIO3_BASE
	MOV		r8, GPIO_SETDATAOUT
	MOV		r9, GPIO_CLRDATAOUT
	//SET BP0
SET_BP0:
	QBBC	CLEAR_BP0, r2, 0
	MOV		r0, (1<<18)
	SBBO 	r0, r5, r8, 4
	JMP 	SET_BP1
CLEAR_BP0:	
	MOV		r0, (1<<18)
	SBBO	r0, r5, r9, 4
		
SET_BP1:
	QBBC	CLEAR_BP1, r2, 1
	MOV		r0, (1<<12)
	SBBO 	r0, r5, r8, 4
	JMP 	SET_BP2
CLEAR_BP1:	
	MOV		r0, (1<<12)
	SBBO	r0, r5, r9, 4

SET_BP2:
	QBBC	CLEAR_BP2, r2, 2
	MOV		r0, (1<<3)
	SBBO 	r0, r6, r8, 4
	JMP 	SET_BP3
CLEAR_BP2:	
	MOV		r0, (1<<3)
	SBBO	r0, r6, r9, 4
	
SET_BP3:
	QBBC	CLEAR_BP3, r2, 3
	MOV		r0, (1<<4)
	SBBO 	r0, r6, r8, 4
	JMP 	SET_SS0
CLEAR_BP3:	
	MOV		r0, (1<<4)
	SBBO	r0, r6, r9, 4
	
SET_SS0:
	QBBC	CLEAR_SS0, r3, 0
	MOV		r0, (1<<5)
	SBBO 	r0, r6, r8, 4
	JMP 	SET_SS1
CLEAR_SS0:	
	MOV		r0, (1<<5)
	SBBO	r0, r6, r9, 4
	
SET_SS1:
	QBBC	CLEAR_SS1, r3, 1
	MOV		r0, (1<<2)
	SBBO 	r0, r6, r8, 4
	JMP 	SET_SS2
CLEAR_SS1:	
	MOV		r0, (1<<2)
	SBBO	r0, r6, r9, 4
	
SET_SS2:
	QBBC	CLEAR_SS2, r3, 2
	MOV		r0, (1<<19)
	SBBO 	r0, r5, r8, 4
	JMP 	SET_SS3
CLEAR_SS2:	
	MOV		r0, (1<<19)
	SBBO	r0, r5, r9, 4
	
SET_SS3:
	QBBC	CLEAR_SS3, r3, 3
	MOV		r0, (1<<28)
	SBBO 	r0, r5, r8, 4
	JMP 	END_SELECT_MODULE
CLEAR_SS3:	
	MOV		r0, (1<<28)
	SBBO	r0, r5, r9, 4

END_SELECT_MODULE:	
	RET	
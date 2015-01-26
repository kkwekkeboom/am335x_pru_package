#ifndef _PRU_H
#define _PRU_H

// ***************************************
// *      Global Macro definitions       *
// ***************************************

#define PRU_NUM 0

#define MODULES_MAX 16 //max 16 modules
#define REQUEST_SIZE_MAX 1 //currently only one byte request size
#define RESPONSE_SIZE_MAX 256 //max 256 bytes send from spi module

#define SLAVE_SEL1 "/sys/class/gpio/gpio69/value"
#define SLAVE_SEL2 "/sys/class/gpio/gpio66/value"
#define SLAVE_SEL3 "/sys/class/gpio/gpio51/value"
#define SLAVE_SEL4 "/sys/class/gpio/gpio60/value"
#define BACKPLANE_SEL1 "/sys/class/gpio/gpio50/value"
#define BACKPLANE_SEL2 "/sys/class/gpio/gpio44/value"
#define BACKPLANE_SEL3 "/sys/class/gpio/gpio67/value"
#define BACKPLANE_SEL4 "/sys/class/gpio/gpio68/value"
#define RESET_PIN "/sys/class/gpio/gpio86/value"

#define PRU0_ARM_INTERRUPT      19
#define PRU1_ARM_INTERRUPT      20

#define CONST_PRUCFG    		C4
#define CONST_PRUDRAM   		C24
#define CONST_PRUSHAREDRAM   	C28
#define CONST_L3RAM     		C30
#define CONST_DDR            	C31

// Address for the Constant table Programmable Pointer Register 0(CTPPR_0)
#define CTBIR_0         0x22020
// Address for the Constant table Programmable Pointer Register 0(CTPPR_0)
#define CTBIR_1         0x22024

// Address for the Constant table Programmable Pointer Register 0(CTPPR_0)
#define CTPPR_0         0x22028

// Address for the Constant table Programmable Pointer Register 1(CTPPR_1)
#define CTPPR_1         0x2202C

//PRU_ICSS_CFG registers
#define REVID			0x00
#define SYSCFG_OFFSET 	0x04
#define GPCFG0			0x08
#define GPCFG1			0x0C
#define CGR				0x10
#define ISPR			0x14
#define ISP				0x18
#define IESP			0x1C
#define IECP			0x20
#define PMAO			0x28
#define IEPCLK			0x30
#define SPP				0x34
#define PIN_MX			0x40


//SPI CLOCK management registers
#define SPI0_CLOCK		0x44E0004C
#define SPI1_CLOCK		0x44E00050

//BASE registers
#define SPI0_BASE		0x48030000
#define SPI1_BASE		0x481A0000
#define GPIO0_BASE		0x44E07000
#define GPIO1_BASE		0x4804C000
#define GPIO2_BASE		0x481AC000
#define GPIO3_BASE		0x481AE000

#define PRU_INTC_BASE	0x00020000

//SPI Register offsets
#define MCSPI_REVISION 	0
#define MCSPI_SYSCONFIG 0x110
#define MCSPI_SYSSTATUS 0x114
#define MCSPI_IRQSTATUS	0x118
#define MCSPI_IRQENABLE 0x11C
#define MCSPI_WAKEUPENABLE 0x120
#define MCSPI_SYST		0x124
#define MCSPI_MODULECTRL 0x128
#define MCSPI_CH0CONF	0x12C
#define MCSPI_CH0STAT	0x130
#define MCSPI_CH0CTRL	0x134
#define MCSPI_TX0		0x138
#define MCSPI_RX0		0x13C
#define MCSPI_CH1CONF	0x140
#define MCSPI_CH1STAT	0x144
#define MCSPI_CH1CTRL	0x148
#define MCSPI_TX1		0x14C
#define MCSPI_RX1		0x150
#define MCSPI_CH2CONF	0x154
#define MCSPI_CH2STAT	0x158
#define MCSPI_CH2CTRL	0x15C
#define MCSPI_TX2		0x160
#define MCSPI_RX2		0x164
#define MCSPI_CH3CONF	0x168
#define MCSPI_CH3STAT	0x16C
#define MCSPI_CH3CTRL	0x170
#define MCSPI_TX3		0x174
#define MCSPI_RX3		0x178
#define MCSPI_XFERLEVEL	0x17C


//MCSPI WAKEUPENABLE (undocumented in manual)
#define WKEN			0

//MCSPI MODULECTRL register
#define SINGLE_CHANNEL	0
#define	CS_DISABLE 		1
#define MS				2
#define SYSTEST			3
#define INIT_DELAY		4
#define MOA				7
#define FIFO_DMA		8

//MCSPI SYSCONFIG register
#define AUTOIDLE		0
#define	SOFTRESET		1
#define SIDLEMODE		3
#define CLOCK_ACTIVITY	8

//MCSPI SYST register
#define SPIEN0			0
#define	SPIEN1			1
#define	SPIEN2			2
#define	SPIEN3			3
#define	SPIDAT0			4
#define	SPIDAT1			5
#define SPICLK			6
#define SPIDATDIR0		8
#define SPIDATDIR1		9
#define SPIENDIR		10
#define	SSB				11

//MCSPI CH(i)CFG register definitions
#define SPI_CPHA		0                    /* clock phase */
#define SPI_CPOL		1                    /* clock polarity */
#define CLKD			2
#define EPOL 			6
#define WL				7
#define TRM0			12
#define	TRM1			13
#define DMAW			14
#define DMAR			15
#define DEP0			16
#define	DEP1			17
#define IS				18
#define TURBO			19
#define FORCE			20
#define SPIENSLV		21
#define	SBE				23
#define SBPOL			24
#define TCS				25
#define FFEW			27
#define FFER			28
#define CLKG			29
#define SPI_MODE_0      (0|0)                   /* (original MicroWire) */
#define SPI_MODE_1      (0|1<<SPI_CPHA)
#define SPI_MODE_2      (1<<SPI_CPOL|0)
#define SPI_MODE_3      (1<<SPI_CPOL| 1 <<SPI_CPHA)

//MCSPI CH(i)CTRL
#define EN				0
#define EXTCLK			8


//GPIO_REGISTERS OFFSET
#define GPIO_SETDATAOUT	0x194
#define GPIO_CLRDATAOUT	0x190


#define	BYTE_COUNTER	r20 //stores which byte is currently read by the module
#define MESSAGE_LENGTH	r21 //stores the maximum byte length of a spi message, which is 256 or the 2nd byte of the SPI message
#define	REPLY_MEM_OFFSET r22 //stores address in PRU_DATARAM where a reply begins
#define	REPLY_MEM_POINTER	r23 //stores current position in reply
#define	COMMAND_MEM_POINTER r24 //stores the current command position
#define	SPI1_OFFSET		r25
#define MODULE_COUNTER	r26 //stores which module is currently read
#define MODULES_LENGTH	r27 //stores the maximum number of modules
#define	COMMAND			r28

//bytes in the SPI_MESSAGE
#define START_BYTE 		0x00000023 //start byte = #
#define REQUEST_CMD 	0x000000A1 // Request command for I/O modules
#define REQUEST_CMD2 	0x000000A3 // Request command for I/O modules
#define REQUEST_STOP 	0x0000000A2 // End of request byte
#define RESET_CMD 		0x0000000D1 //reset command for initialization
#define INITIALIZING 0x66 //IO module is initializing

#endif // _SPI_HP
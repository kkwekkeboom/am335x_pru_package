// Standard header files
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

/******************************************************************************
* Local Macro Declarations                                                    * 
******************************************************************************/
#define PRU_NUM 	0
#define DEVICE "/dev/spidev1.0"
#define SLAVE_SEL1 "/sys/class/gpio/gpio69/value"
#define SLAVE_SEL2 "/sys/class/gpio/gpio66/value"
#define SLAVE_SEL3 "/sys/class/gpio/gpio51/value"
#define SLAVE_SEL4 "/sys/class/gpio/gpio60/value"
#define BACKPLANE_SEL1 "/sys/class/gpio/gpio50/value"
#define BACKPLANE_SEL2 "/sys/class/gpio/gpio44/value"
#define BACKPLANE_SEL3 "/sys/class/gpio/gpio67/value"
#define BACKPLANE_SEL4 "/sys/class/gpio/gpio68/value"
#define RESET_PIN "/sys/class/gpio/gpio86/value"
#define NR_SELECT_LINES 9

#define SPI_MODE_1 		(1<<1)
#define CLK_0			((0<<2) | (0<<3) | (0<<4) | (0<<5))
#define EPOL 			(1<<6)
#define WL8				(7<<7)
#define TRM_TX_RX		(0<<12)
#define NO_DMA			(0<<14 | 0 << 15)
#define DATA_LINES		(9<<16)
#define FORCE			(1<<20)
#define MCSPI_CH1CFG	(SPI_MODE_1 | CLK_0 | EPOL | WL8 | TRM_TX_RX | NO_DMA | DATA_LINES | FORCE)

/******************************************************************************
* Local Function Declarations                                                 *
******************************************************************************/

static int LOCAL_exampleInit ( );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );
void selectIOModule(unsigned int id);
void setPin(int fd, int value);

/******************************************************************************
* Global Variable Definitions                                                 *
******************************************************************************/

static void *pruDataMem;
static unsigned int *pruDataMem_int;
static unsigned int pins[NR_SELECT_LINES];

/*****************************************************************************
* Global Function Definitions                                                 *
*****************************************************************************/



/******************************************************************************
* Main                                                                        *
******************************************************************************/
int main (int argc, char* argv[])
{
	int ret;
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	prussdrv_init();

	ret = prussdrv_open(PRU_EVTOUT_0);
	if (ret < 0)
	{
		printf("\tERROR: prussdrv_open failed\n");
		return (ret);
	}
	prussdrv_pruintc_init(&pruss_intc_initdata);
	LOCAL_exampleInit();
	prussdrv_exec_program (PRU_NUM, "./PRU_spidriver.bin");
	prussdrv_pru_wait_event (PRU_EVTOUT_0); //wait for halt
	prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
	if ( LOCAL_examplePassed(PRU_NUM) )
	{
		printf("Example executed succesfully.\r\n");
	}
	else
	{
		printf("Example failed.\r\n");
	}
	prussdrv_pru_disable(PRU_NUM);
	prussdrv_exit();
	return 0;
}
/*****************************************************************************
* Local Function Definitions                                                 *
*****************************************************************************/

static int LOCAL_exampleInit (  )
{
	int n = 0, i,k;
	 //Initialize pointer to PRU data memory
	prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pruDataMem);

	pruDataMem_int = (unsigned int*) pruDataMem;

	// Flush the values in the PRU data memory locations
	pruDataMem_int[0] = 0x40;
	pruDataMem_int[1] = 0x00;
	pins[n++] = open(RESET_PIN, O_WRONLY);
	pins[n++] = open(SLAVE_SEL1, O_WRONLY);
	pins[n++] = open(SLAVE_SEL2, O_WRONLY);
	pins[n++] = open(SLAVE_SEL3, O_WRONLY);
	pins[n++] = open(SLAVE_SEL4, O_WRONLY);
	pins[n++] = open(BACKPLANE_SEL1, O_WRONLY);
	pins[n++] = open(BACKPLANE_SEL2, O_WRONLY);
	pins[n++] = open(BACKPLANE_SEL3, O_WRONLY);
	pins[n++] = open(BACKPLANE_SEL4, O_WRONLY);
	setPin(pins[0], 1); //reset modules
	k = MCSPI_CH1CFG;
	printf("here %x\n", MCSPI_CH1CFG);
	usleep(1000);
	/*for (i= 0; i < 16; i++)
	{
		selectIOModule(i);
		usleep(500000);
	}*/
	selectIOModule(1);
	return(0);
}

static unsigned short LOCAL_examplePassed ( unsigned short pruNum )
{
	int i;
	for (i = 0; i < 5; i++)
	{
		printf("[%d]", pruDataMem_int[i]);
	}
	printf("\n");

	//{
//		printf("result%d=%d",i,pruDataMem_int[i]);
	//}
	return 1;
}


void setPin(int fd, int value)
{
	if(value)
	{
		write(fd, "1", 1);
	}
	else
	{
		write(fd, "0", 1);
	}
}

/**
 * @function selectIOModule Set select lines to talk to only one io module
 * @param	id Id of IO module (0-15)
 */

void selectIOModule(unsigned int id)
{
	unsigned int backplaneMask = (1 << (int) (id / 4));
	unsigned int moduleMask = (1 << (id % 4));

	printf("Select backplane %d module %d\n", backplaneMask, moduleMask);
	setPin(pins[0], 0); //don't use reset
	setPin(pins[1], moduleMask & (1<<0));
	setPin(pins[2], moduleMask & (1<<1));
	setPin(pins[3], moduleMask & (1<<2));
	setPin(pins[4], moduleMask & (1<<3));

	setPin(pins[5], backplaneMask & (1<<0));
	setPin(pins[6], backplaneMask & (1<<1));
	setPin(pins[7], backplaneMask & (1<<2));
	setPin(pins[8], backplaneMask & (1<<3));
}

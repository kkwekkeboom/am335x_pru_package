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
#define PIN	 	 1<<15

/******************************************************************************
* Local Function Declarations                                                 *
******************************************************************************/

static int LOCAL_exampleInit ( );
static unsigned short LOCAL_examplePassed ( unsigned short pruNum );

/******************************************************************************
* Global Variable Definitions                                                 *
******************************************************************************/

static void *pruDataMem;
static unsigned int *pruDataMem_int;

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
	 //Initialize pointer to PRU data memory
	prussdrv_map_prumem (PRUSS0_PRU0_DATARAM, &pruDataMem);

	pruDataMem_int = (unsigned int*) pruDataMem;

	// Flush the values in the PRU data memory locations
	pruDataMem_int[0] = 0xffff;
	pruDataMem_int[1] = 0x32;
	return(0);
}

static unsigned short LOCAL_examplePassed ( unsigned short pruNum )
{
	printf("result1 %d\n", pruDataMem_int[0]);
	printf("result2 %d\n", pruDataMem_int[1]);
	return 1;
}

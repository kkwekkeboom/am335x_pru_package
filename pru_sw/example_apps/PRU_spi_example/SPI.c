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
#define OFFSET_SHAREDRAM 2048		//equivalent with 0x00002000

#define PRUSS0_SHARED_DATARAM    4

/******************************************************************************
* Global variable Declarations                                                * 
******************************************************************************/
static void *sharedMem;
static unsigned int *sharedMem_int;

static int freqArray[16][2] = { 
{48000000,0x00000000},                               
{24000000,0x00000004},
{12000000,0x00000008},
{ 6000000,0x0000000C},
{ 3000000,0x00000010},
{ 1500000,0x00000014},
{  750000,0x00000018},
{  375000,0x0000001C},
{  187500,0x00000020},
{   93750,0x00000024},
{   46875,0x00000028},
{   23437,0x0000002C},
{   11718,0x00000030},
{    5859,0x00000034},
{    2929,0x00000038},
{    1464,0x0000003C} };

int validate (int, char*);

/******************************************************************************
* Main                                                                        * 
******************************************************************************/
int main (int argc, char* argv[])
{
	int ret;
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	ret = prussdrv_init();
	validate(ret, "init");
	ret = prussdrv_open(PRU_EVTOUT_0);
	validate(ret, "open");
	ret = prussdrv_pruintc_init(&pruss_intc_initdata);
	validate(ret, "pruintc");
	//validate(ret, "exec");
	prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &sharedMem);
	sharedMem_int = (unsigned int*) sharedMem;
	sharedMem_int[OFFSET_SHAREDRAM + 0] = 0xA3;
	sharedMem_int[OFFSET_SHAREDRAM + 1] = 20;
	sharedMem_int[OFFSET_SHAREDRAM + 2] = freqArray[2][1];
	ret = prussdrv_pru_disable(0);
	validate(ret, "disable");
	ret = prussdrv_exec_program (PRU_NUM, "./SPI.bin");
	//ret = prussdrv_pru_write_memory(PRUSS0_PRU0_IRAM, 0, PRUcode, sizeof(PRUcode));
	validate(ret, "write");


	printf("\tINFO: Transmitting %d (%d times) at %d Hz\n", 0xA3, 20, freqArray[2][0]);
	printf("\tINFO: Sending...");

	    //printf("\tINFO: PRU completed transfer.\r\n");
	ret = prussdrv_pru_enable(0);
	validate(ret, "enable");
	ret = prussdrv_pru_wait_event (PRU_EVTOUT_0);
	validate(ret, "wait");
	prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
	/*unsigned int ret, numWords, word, frequency;
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
    	struct timeval start, end;
    	long mtime, seconds, useconds;    

	if(argc != 4){
		printf("\tERROR: Parameters Necessary\n");
		printf("\t       %s [word] [num words] [frequency (0-15)]\n", argv[0]);
		return 0;
	}

	word = atoi(argv[1]);
        numWords = atoi(argv[2]);
        frequency = atoi(argv[3]);

	// Initializing PRU
	prussdrv_init();
	ret = prussdrv_open(PRU_EVTOUT_0);
	if (ret){
        	printf("\tERROR: prussdrv_open open failed\n");
        	return (ret);
    	}

    prussdrv_pruintc_init(&pruss_intc_initdata);
    printf("\tINFO: Initializing.\r\n");
    prussdrv_map_prumem(PRUSS0_SHARED_DATARAM, &sharedMem);
	
    sharedMem_int = (unsigned int*) sharedMem;
    sharedMem_int[OFFSET_SHAREDRAM + 0] = word;
    sharedMem_int[OFFSET_SHAREDRAM + 1] = numWords;
    sharedMem_int[OFFSET_SHAREDRAM + 2] = freqArray[frequency][1];
    
    // Executing PRU.
    prussdrv_exec_program (PRU_NUM, "Test_bin.h");
    //prussdrv_pru_write_memory(PRUSS0_PRU0_IRAM, 0, PRUcode, sizeof(PRUcode));
    //prussdrv_pru_enable(0);

    gettimeofday(&start, NULL);

    printf("\tINFO: Transmitting %d (%d times) at %d Hz\n", word, numWords, freqArray[frequency][0]);
    printf("\tINFO: Sending...");

    //printf("\tINFO: PRU completed transfer.\r\n");
    //prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

   ret = prussdrv_pru_wait_event(PRU_EVTOUT_0);

    gettimeofday(&end, NULL);

    seconds  = end.tv_sec  - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    mtime = (seconds + (useconds/1000000));

    printf("PRU program completed (%d), in %d seconds \n", ret, (int)mtime);
    printf("Return value %d\n", sharedMem_int[OFFSET_SHAREDRAM + 0]);
    prussdrv_pru_clear_event (PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);

    // Disable PRU
    prussdrv_pru_disable(PRU_NUM);
    prussdrv_exit();

    return(0);*/
}

int validate (int ret, char* function)
{
	if (ret < 0)
	{
		printf("\tERROR: %s failed\n", function);
		exit (ret);
	}
	else
	{
		printf("\t %s\n", function);
	}
	return 0;
}

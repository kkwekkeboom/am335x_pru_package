#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
//#include <pthread.h>

#include <signal.h>
#include <fcntl.h>

#include <prussdrv.h>
#include <pruss_intc_mapping.h>
#include "pru.h"

/******************************************************************************
* Local Macro Declarations                                                    *
******************************************************************************/

#ifndef START_ADDR
#error "START_ADDR must be defined"
#endif
#define HWREG(x) (*((volatile unsigned int *)(x)))

/******************************************************************************
* Global Variable Definitions                                                 *
******************************************************************************/

static int finish = 0;
static void *pruDataMem;
static unsigned char *requests;
static unsigned char *responses;
static unsigned int reset_pin;


/******************************************************************************
* Global function Definitions                                                 *
******************************************************************************/

void init_pru_program();

void init_spi_request();

//TODO: move setPin to PRU program
void setPin(int fd, int value);

void exit_handler(int signal);

/******************************************************************************
* Main Program                                                                *
******************************************************************************/

int main(int argc, const char *argv[])
{
	int i, iterations = 4,ret;
   // Listen to SIGINT signals (program termination)
   signal(SIGINT, exit_handler);
   // Load and run binary into pru0
   init_pru_program();
   //ret = HWREG(0x4a32200C);
   //printf("counter %x\n", ret);
   prussdrv_pru_wait_event(PRU_EVTOUT_0);
   prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
   prussdrv_read_program(PRU_NUM, "./text_exec.bin");
   printf("here2\n");
   while(iterations)
   {
      // Initialize spi program request
      init_spi_request();

      // Send interrupt to PRU to execute request
      prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);
      printf("here3\n");

      // Wait for PRU to finish executing request
      prussdrv_pru_wait_event(PRU_EVTOUT_0);
      prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
      printf("here4\n");
      for (i = 0; i < MODULES_MAX; i++)
      {
    	  printf("%d\n", requests[i]);
      }
      iterations--;
   }

   printf("Disabling PRU.\n");
   prussdrv_pru_disable(PRU_NUM);
   prussdrv_exit ();
   return 0;
}


void init_pru_program()
{
	int ret;
	tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;
	//prussdrv_pru_disable(PRU_NUM);
	ret =prussdrv_init();
	printf("ret1%d\n", ret);
	ret =prussdrv_open(PRU_EVTOUT_0);
	printf("ret2%d\n", ret);
	ret =prussdrv_pruintc_init(&pruss_intc_initdata);
	printf("ret3%d\n", ret);
	ret =prussdrv_load_datafile(PRU_NUM, "./data.bin");
	printf("ret4%d\n", ret);
	ret =prussdrv_exec_program_at(PRU_NUM, "./text.bin", START_ADDR);
	printf("ret5%d\n", ret);
}

void init_spi_request()
{
	prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, &pruDataMem);

	requests = (unsigned char*) pruDataMem;
	responses = (unsigned char*) (requests + (MODULES_MAX * REQUEST_SIZE_MAX)); //responses array starts after requests

	// Set the requests and respones to zero
	//memset(requests, 0, sizeof(unsigned char) * MODULES_MAX * REQUEST_SIZE_MAX);
	//memset(responses, 0, sizeof(unsigned char) * MODULES_MAX * REQUEST_SIZE_MAX * RESPONSE_SIZE_MAX);
	//requests[1] = 64;
	//requests[2] = 80;

	reset_pin = open(RESET_PIN, O_WRONLY);
	setPin(reset_pin, 1); //reset modules
	usleep(1000);
	setPin(reset_pin, 0); //reset modules
	usleep(100000);
}

void exit_handler(int signal)
{
   finish = 1;
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

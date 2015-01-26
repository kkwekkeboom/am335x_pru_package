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
	int i, iterations = 0,ret;
   // Listen to SIGINT signals (program termination)
   signal(SIGINT, exit_handler);
   // Load and run binary into pru0
   init_pru_program();
   printf("initialized program\n");
   prussdrv_pru_wait_event(PRU_EVTOUT_0);
   prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
   printf("pru ready\n");
   while(iterations)
   {
      // Initialize spi program request
      init_spi_request();

      // Send interrupt to PRU to execute request
      prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);
      printf("send request to PRU\n");

      // Wait for PRU to finish executing request
      prussdrv_pru_wait_event(PRU_EVTOUT_0);
      prussdrv_pru_clear_event(PRU_EVTOUT_0, PRU0_ARM_INTERRUPT);
      printf("pru completed transfer\n");
      for (i = 0; i < MODULES_MAX; i++)
      {
    	  printf("%d\n", requests[i]);
      }
      iterations--;
      usleep(10);
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
	ret =prussdrv_init();
	ret =prussdrv_open(PRU_EVTOUT_0);
	//ret =prussdrv_pruintc_init(&pruss_intc_initdata);
	//ret =prussdrv_load_datafile(PRU_NUM, "./data.bin");
	//ret =prussdrv_exec_program_at(PRU_NUM, "./text.bin", START_ADDR);
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

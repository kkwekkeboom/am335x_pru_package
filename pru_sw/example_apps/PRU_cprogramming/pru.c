// Macro for accessing a hardware register (32 bit)

#include "pru.h"

/******************************************************************************
 * Local Macro Declarations                                                    *
 ******************************************************************************/

#define HWREG(x) (*((volatile unsigned int *)(x)))
#define SET_BP0(val)	HWREG(GPIO1_BASE + (0 < (val) ? GPIO_SETDATAOUT : GPIO_CLRDATAOUT)) = (1<<18)
#define SET_BP1(val)	HWREG(GPIO1_BASE + (0 < (val) ? GPIO_SETDATAOUT : GPIO_CLRDATAOUT)) = (1<<12)
#define SET_BP2(val)	HWREG(GPIO2_BASE + (0 < (val) ? GPIO_SETDATAOUT : GPIO_CLRDATAOUT)) = (1<<3)
#define SET_BP3(val)	HWREG(GPIO2_BASE + (0 < (val) ? GPIO_SETDATAOUT : GPIO_CLRDATAOUT)) = (1<<4)
#define SET_SS0(val)	HWREG(GPIO2_BASE + (0 < (val) ? GPIO_SETDATAOUT : GPIO_CLRDATAOUT)) = (1<<5)
#define SET_SS1(val)	HWREG(GPIO2_BASE + (0 < (val) ? GPIO_SETDATAOUT : GPIO_CLRDATAOUT)) = (1<<2)
#define SET_SS2(val)	HWREG(GPIO1_BASE + (0 < (val) ? GPIO_SETDATAOUT : GPIO_CLRDATAOUT)) = (1<<19)
#define SET_SS3(val)	HWREG(GPIO1_BASE + (0 < (val) ? GPIO_SETDATAOUT : GPIO_CLRDATAOUT)) = (1<<28)
#define DEBUG_PIN (1<<5)

/******************************************************************************
 * Global variable defitions                                                   *
 *******************************************************************************/
unsigned char* requests;
unsigned char* responses;

/******************************************************************************
 * Global function Definitions                                                 *
 ******************************************************************************/
inline int getARMInterrupt();
inline void createARMInterrupt();
inline void clearInterrupt();

void selectModule(int module);
int handleRequest();

/******************************************************************************
 * Global flags                                                                *
 ******************************************************************************/

volatile register unsigned int __R30;
volatile register unsigned int __R31;

int main(int argc, const char *argv[]){
	// PRU DRAM is at address 0x10000.
	short i = 600;

	// Enable OCP master port
	HWREG(0x26004) &= (1<<4);

	// Set PINMUX for DEBUG pin
	HWREG(0x26040) |= (1<<1);

	// Clear interrupts
	clearInterrupt();

	// Initialize data
	requests = (unsigned char *) 0x00000;
	responses = (unsigned char *) (requests + (MODULES_MAX * REQUEST_SIZE_MAX));
	// Writing to register 31 sends interrupt requests to the
	// ARM system. See section 4.4.1.2.2 Event Interface
	// Mapping (R31): PRU System Events in page 209 of manual.
	// Not sure why vector output is 4 in this case but this is
	// what they do in
	__R30 &= ~DEBUG_PIN;
	while(i--);
	__R30 |= DEBUG_PIN;
	createARMInterrupt();


	while(1){
		//count ++;
		// If bit 30 of register 31 is set. That means someone sent
		// an interrupt request to this PRU.
		// ISRs do not exist for the PRU (design choice)
		if(getARMInterrupt())
		{
			__R30 &= ~DEBUG_PIN;
			clearInterrupt();
			i =600;
			while(i--);
			//for (i = 0; i < MODULES_MAX; i++)
				//selectModule(1);
			__R30 |= ~DEBUG_PIN;
			createARMInterrupt();

		}
	}
	// stop pru processing
	//__halt();

	return 0;
}

inline int getARMInterrupt()
{
	return (__R31 & (1 << 30));
}


inline void createARMInterrupt()
{
	__R31 = 35;
	return;
}

inline void clearInterrupt()
{
	// Writing to the SICR register clears the interrupt
	// request. The address of the register is 0x20024:
	// 0x20000 is the base address for INTC (interrupt
	// controller) registers (page 204) and 0x24 is the
	// offset for the SICR register (page 322, Table 4-102.
	// PRU_ICSS_INTC Registers.
	// The value 21 is the event number sent from the ARM
	// (linux) code. Again, not sure why they chose this
	// number but that's what they do in other examples.
	HWREG(0x20024) = 21;
}

void selectModule(int module)
{

	unsigned int backplaneMask = (1 << (int)(module / 4));
	unsigned int moduleMask = (1 << (module % 4));

	SET_BP0(moduleMask & (1<<0));
	SET_BP1(moduleMask & (1<<1));
	SET_BP2(moduleMask & (1<<2));
	SET_BP3(moduleMask & (1<<3));

	SET_SS0(backplaneMask & (1<<0));
	SET_SS0(backplaneMask & (1<<1));
	SET_SS0(backplaneMask & (1<<2));
	SET_SS0(backplaneMask & (1<<3));
}

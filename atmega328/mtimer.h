/********************************************************************************
Includes
********************************************************************************/
#include <avr/io.h>
#include <stdio.h>

/********************************************************************************
	Macros and Defines
********************************************************************************/
#define _in(bit,port)	port &= ~(1 << bit)
#define _out(bit,port)	port |= (1 << bit)
#define _on(bit,port)	port |= (1 << bit)
#define _off(bit,port)	port &= ~(1 << bit)

#define _to_uint64(x,y) ((uint64_t) x << 16) | y

/********************************************************************************
Function Prototypes
********************************************************************************/
void initTimer();
void incrementOvf();
uint64_t convertSecondsToCicles(uint16_t value);
uint64_t getCurrentTimeCicles();
uint32_t getElapsedMilliseconds(uint64_t startTime);

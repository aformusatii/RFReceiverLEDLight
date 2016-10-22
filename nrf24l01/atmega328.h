/********************************************************************************
Includes
********************************************************************************/
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

/********************************************************************************
Macros and Defines
********************************************************************************/
#define _in(bit,port)	port &= ~(1 << bit)
#define _out(bit,port)	port |= (1 << bit)
#define _on(bit,port)	port |= (1 << bit)
#define _off(bit,port)	port &= ~(1 << bit)

#define SPI_CSN PORTB1
#define SPI_CE  PORTB2

/* =========== SPI and GPIO function ============ */
void setup_io();
void setup_spi();
void setCSN(uint8_t value);
void setCE(uint8_t value);
uint8_t transfer_spi(uint8_t tx_);

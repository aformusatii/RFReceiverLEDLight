/********************************************************************************
	Includes
********************************************************************************/
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <util/delay.h>
#include <stdlib.h>
#include <avr/eeprom.h>

#include "../nrf24l01/RF24.h"
#include "../common/util.h"
#include "../nrf24l01/atmega328.h"
#include "../atmega328/mtimer.h"

extern "C" {
#include "../atmega328/usart.h"
}

/********************************************************************************
	Macros and Defines
********************************************************************************/

/********************************************************************************
	Function Prototypes
********************************************************************************/
void initGPIO();

/********************************************************************************
	Global Variables
********************************************************************************/
RF24 radio;
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
volatile uint8_t led_off_delay = 0;
volatile uint64_t switchOffLEDJobCicles = 0;
bool saveLEDOffDelay = false;
bool switchOnWithDelayEnabled = true;
bool switchOnWithDelay = false;

/********************************************************************************
	Interrupt Service
********************************************************************************/
ISR(USART_RX_vect)
{
	handle_usart_interrupt();
}

ISR(INT0_vect)
{
    bool tx_ok, tx_fail, rx_ok;
    radio.whatHappened(tx_ok, tx_fail, rx_ok);

    if (rx_ok) {
        uint8_t data[50];
        uint8_t len = radio.getDynamicPayloadSize();
        radio.read(data, len);

        if (data[0] == 177) {
    		switch (data[1]) {
    			// ======= Switch ON LED Indefinitely ===========
    			case 100:
    				switchOnWithDelayEnabled = false;
    				_on(PC5, PORTC);
    				debug_print("ON LED IND");
    				break;

    			// ======= Switch OFF LED Indefinitely ==========
    			case 101:
    				switchOnWithDelayEnabled = false;
    				_off(PC5, PORTC);
    				debug_print("OFF LED IND");
    				break;

    			// ======= Switch ON LED With Delay =============
    			case 102:
    				switchOnWithDelay = true;
    				break;

    			// ======= Set LED delay ========================
    			case 103:
    				led_off_delay = data[2];
    				debug_print("Set LED delay = %d", led_off_delay);
    				break;

    			// ======= Save current LED delay to EEPROM =====
    			case 104:
    				saveLEDOffDelay = true;
    				debug_print("Save Delay");
    				break;

				// ======= Set & Save LED delay ========================
				case 105:
					led_off_delay = data[2];
					saveLEDOffDelay = true;
					debug_print("Set & Save LED delay = %d", led_off_delay);
					break;

    			// ======= Switch OFF PIR Listener ==============
    			case 106:
    				switchOnWithDelayEnabled = false;
    				debug_print("PIR OFF");
    				break;

    			// ======= Switch ON PIR Listener ===============
    			case 107:
    				switchOnWithDelayEnabled = true;
    				debug_print("PIR ON");
    				break;

    			// ======= Unknown ==============================
    			default:
    				debug_print("Unknown [%d] code", data[1]);
    				break;
    		}
        } else {
        	debug_print("Unknown [%d] main code", data[0]);
        }

        radio.flush_rx();
    }
}

ISR(TIMER1_OVF_vect)
{
	incrementOvf();
}

/********************************************************************************
	Main
********************************************************************************/
int main(void) {
    // initialize usart module
	usart_init();

    // Init GPIO
    initGPIO();

    initTimer();

    // enable interrupts
    sei();

	// Console friendly output
    printf(CONSOLE_PREFIX);

    radio.begin();
    radio.setRetries(15, 15);
    radio.setPayloadSize(8);
    radio.setPALevel(RF24_PA_MAX);
	radio.setChannel(120);

    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1, pipes[1]);

    radio.startListening();

    radio.printDetails();

    // Read saved led off delay from EEPROM
    led_off_delay = eeprom_read_byte((uint8_t *) 0);
    debug_print("LED OFF Delay = %d", led_off_delay);

	// main loop
    while (1) {
    	uint64_t currentTimeCicles = getCurrentTimeCicles();

    	// ================= main usart loop for console ========================
    	usart_check_loop();

    	// ================= Save led off delay to EEPROM =======================
    	if (saveLEDOffDelay) {
    		saveLEDOffDelay = false;
    		eeprom_write_byte((uint8_t *) 0, led_off_delay);
    		debug_print("Saved delay to EEPROM!");
    	}

    	// ================= Switch ON With Delay If Enabled ====================
    	if (switchOnWithDelayEnabled) {

        	// Switch ON LED with delay
        	if (switchOnWithDelay) {
        		switchOnWithDelay = false;
    			switchOffLEDJobCicles = convertSecondsToCicles(60 * led_off_delay);
    			_on(PC5, PORTC);
    			debug_print("Switch ON LED DELAY");
        	}

       	    // Job: Switch OFF LED
        	if ((switchOffLEDJobCicles != 0) && (currentTimeCicles >= switchOffLEDJobCicles)) {
        		switchOffLEDJobCicles = 0;
        		_off(PC5, PORTC);
        		debug_print("Switch OFF LED after delay");
        	}

    	} else {
    		switchOnWithDelay = false;
    	}

    }
}

/********************************************************************************
	Functions
********************************************************************************/
void initGPIO() {
    _in(DDD2, DDRD); // INT0 input

    // GPIO Interrupt INT0
    // The falling edge of INT0 generates an interrupt request.
    EICRA = (0<<ISC11)|(0<<ISC10)|(1<<ISC01)|(0<<ISC00);
    // Enable INT0
    EIMSK = (1<<INT0);

    _out(DDC5, DDRC); // LED SWITCH
    _off(PC5, PORTC);
}


void handle_usart_cmd(char *cmd, char *args) {
	if (strcmp(cmd, "test") == 0) {
		printf("\n TEST [%s]", args);
	}
}

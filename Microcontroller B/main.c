//*********************************************************************************
//* Controller B
//*By James Douglas and Kostadin Pendev
//*
//*Transmits the proper “dose” 8-bit codes over the UART once a minute
//* - currently has determined that 
//*   the patient needs a 0.56 second dose (14 = 0x0E)
//*
//*Safety Features:
//*	-emergency manual dose button
//* -Sends a status request (0xFF) once a second.
//* -Active watchdog to avoid getting stuck in a loop or interrupt
//* -if everything is normal: Green LED
//* -if under 25 seconds, but over 5 seconds of dosage: Red LED
//* -if under 5 seconds of dosage: Blinking Red LED
//* -if not enough insulin left for next dosage: exclamation point on LCD
//* -the LCD displays a message showing if the pump is on or off
//*
//*Honor Code: I have neither given or received, nor have I tolerated
//*				others' use of unauthorized aid.
//*				--James Douglas and Kostadin Pendev					
//*********************************************************************************

// Preprocessor Section (includes and defines)
#include <msp430.h>
#include <math.h>
#include <driverlib.h>
#include <msp430.h>
#include <string.h>
#include "myGpio.h"
#include "myClocks.h"
#include "myLcd.h"

#define ENABLE_PINS 0xFFFE
#define UP 0x0010
#define ACLK 0x0100
#define TACLR 0x0004
#define DIVIDE8 0x00C0

#define ENABLE_PINS 0xFFFE // Required to use inputs and outputs
#define UART_CLK_SEL 0x0080 // Specifies accurate SMCLK clock for UART
#define BR0_FOR_9600 0x34 // Value required to use 9600 baud
#define BR1_FOR_9600 0x00 // Value required to use 9600 baud
#define CLK_MOD 0x4911 // Microcontroller will "clean-up" clock signal

void select_clock_signals(void); // Assigns microcontroller clock signals
void assign_pins_to_uart(void); // P4.2 is for TXD, P4.3 is for RXD
void use_9600_baud(void); // UART operates at 9600 bits/second
void Delay();
void DisplayWord(char word[6]); // Displays words (6 characters or less)

main() {
	WDTCTL = WDTPW | WDTHOLD; // Stop WDT

	PM5CTL0 = ENABLE_PINS; // Enable inputs and outputs

    initGPIO(); // Initialize Inputs and Outputs
    initClocks(); // Initialize clocks
    myLCD_init(); // Initialize LCD

	select_clock_signals(); // Assigns microcontroller clock signals
	assign_pins_to_uart(); // P4.2 is for TXD, P4.3 is for RXD
	use_9600_baud(); // UART operates at 9600 bits/second


	_BIS_SR(GIE); // Activate interrupts


	TA0CCR0 = 8305;               //Assigning the value to count up to 1 second
								  //operating at 1.204E-4 seconds
	TA0CTL = UP + ACLK + TACLR;  // Setting clocks in UP mode and starting count


	TA1CCR0 = 62292;               //Assigning the value to count up to 1 min
								  //operating at 9.632E-4 seconds
	TA1CTL = UP + ACLK + TACLR + DIVIDE8;   // Setting clocks in UP mode, divide by 8, and starting count

	//watchdog timer
	TA2CCR0 = 10; //10ms
	TA2CCTL0 = 0x0010; //enable interrupt
	TA2CTL = UP + ACLK + TACLR;

	P1DIR = 0x00;
	P1DIR = BIT0; // Make P1.0 an output for red LED
	P1OUT = 0x00; // Red LED initially off

	P9DIR = 0x80;            // Make P9.7 an output for green LED
	P9OUT = P9OUT & ~0x80;  // Green LED initially off

    P1OUT = P1OUT | 0x04; // P1.2 will be input with a pull-up
    P1REN = P1REN | 0x04; // resistor.

	P1IFG = 0x00; //clear pending flags
	_BIS_SR(GIE); // Activate interrupts previously enabled

	WDTCTL = 0x5A0C; // start WDT

	TA0CTL = UP + ACLK + TACLR;  // Setting clocks in UP mode and starting count
	TA1CTL = UP + ACLK + TACLR + DIVIDE8;   // Setting clocks in UP mode, divide by 8, and starting count
	TA2CTL = UP + ACLK + TACLR;

	char message=0x01; //receive message////////////////////////////////////////////////////////////////////////

	while (1) // Checking for incoming messages
	{


		if(UCA0IFG & UCRXIFG){ // Received any new messages?
			message=UCA0RXBUF;
			UCA0IFG = UCA0IFG & (~UCRXIFG); // Reset the UART receive flag
		}


		// if under 25 seconds, but over 5 seconds of dosage
		if ((message==0x02) || (message==0x89)) {

			P1OUT |= BIT0; // Then, turn on red LED
			P9OUT= P9OUT &~ 0x80;

		}else if ((message==0x88) || (message==0x01)){// Normal status Green LED on
		    myLCD_showSymbol(LCD_CLEAR, LCD_EXCLAMATION,0); //remove the exclamation point
			P9OUT = P9OUT | 0x80; //turn on green LED
			P1OUT &= ~BIT0; // Turn off the red LED                                  
		}


		if (TA0CTL & 0x0001) {                   //send status request every second
			UCA0TXBUF = 0xFF;                    // Send the UART message out pin P4.2

			//if dosage under 5sec
			if ((message==0x03) || (message==0x8A)) {
				P1OUT = P1OUT ^ 0x01;      //Toggle Red LED
				P9OUT = P9OUT & ~0x80; //turn off green LED
			}

			TA0CTL = UP + ACLK + TACLR;          //clear flag
		}

		if (TA1CTL & 0x0001) {                   //send dosage every min
			UCA0TXBUF = 0x14;                    // Send the UART message out pin P4.2
			TA1CTL = UP + ACLK + TACLR;          //clear flag
		}


		if ((message==0x01) || (message==0x02) || (message==0x03)) {
			DisplayWord("P OFF "); // Display word in double quotes on LCD
		}else{
			DisplayWord(" P ON "); // Display word in double quotes on LCD
		}
		//Alert Message
		if (message==0xFF) {
		    myLCD_showSymbol(LCD_UPDATE, LCD_EXCLAMATION,0); //add the exclamation point
		}


	}// end while(1)
}


#pragma vector=TIMER2_A0_VECTOR
__interrupt void Timer2_ISR (void)
{
    WDTCTL = 0x5A0C; //pet watchdog

    //button debounce and sample code

    //last state of the button
    static int lastPress = BIT2;

    //count of consecutive samples opposing the last state
    static int count = 0;

    //if we get an opposing sample, add it, otherwise reset count to 0
    count = ((P1IN ^ lastPress) & BIT2) ? count+1 : 0;

    //if we get 5 opposing samples in a row, toggle state and reset count
    if(count>=5){
        count = 0;
        lastPress ^= BIT2;
        if(lastPress ==0)UCA0TXBUF = 0x3E;  //send pump request if pressed
    }


}

//*********************************************************************************
//* Select Clock Signals *
//*********************************************************************************
void select_clock_signals(void) {
CSCTL0 = 0xA500; // "Password" to access clock calibration registers
CSCTL1 = 0x0046; // Specifies frequency of main clock
CSCTL2 = 0x0133; // Assigns additional clock signals
CSCTL3 = 0x0000; // Use clocks at intended frequency, do not slow them down
}

//*********************************************************************************
//* Used to Give UART Control of Appropriate Pins *
//*********************************************************************************
void assign_pins_to_uart(void) {
P4SEL1 = 0x00;        // 0000 0000
P4SEL0 = BIT3 | BIT2; // 0000 1100
					  // ^^
					  // |+---- 01 assigns P4.2 to UART Transmit (TXD)
					  // |
					  // +----- 01 assigns P4.3 to UART Receive (RXD)
}

//*********************************************************************************
//* Specify UART Baud Rate *
//*********************************************************************************
void use_9600_baud(void) {
UCA0CTLW0 = UCSWRST; // Put UART into SoftWare ReSeT
UCA0CTLW0 = UCA0CTLW0 | UART_CLK_SEL; // Specifies clock source for UART
UCA0BR0 = BR0_FOR_9600; // Specifies bit rate (baud) of 9600
UCA0BR1 = BR1_FOR_9600; // Specifies bit rate (baud) of 9600
UCA0MCTLW = CLK_MOD; // "Cleans" clock signal
UCA0CTLW0 = UCA0CTLW0 & (~UCSWRST); // Takes UART out of SoftWare ReSeT
}

//*********************************************************************************
//* DisplayWord() - Used to display a word up to 6 characters on the LCD
//*********************************************************************************
void DisplayWord(char word[6])
{
    unsigned int length; // Used to store length of word
    unsigned int i; // Used to "step" through word, 1 character at a time
    char next_char; // The character in word presently displaying
    length = strlen(word); // Get length of the desired word
    if (length <= 6) // If 6 or less characters
    {
        for (i = 0; i <= length - 1; i = i + 1) // Loop through each of characters
        {
            next_char = word[i]; // Get character for the ith slot
            if (next_char) // If character exists (not null)
            {
                myLCD_showChar(next_char, i + 1); // Show character on LCD
            }
        }
    }
    else // Else, word has more than 6 characters, display error message
    {
        myLCD_showChar('E', 1);
        myLCD_showChar('R', 2);
        myLCD_showChar('R', 3);
        myLCD_showChar('O', 4);
        myLCD_showChar('R', 5);
        myLCD_showChar(' ', 6);
    }
}

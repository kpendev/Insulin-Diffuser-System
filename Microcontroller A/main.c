//*********************************************************************************
//* Controller A
//*By James Douglas and Kostadin Pendev
//*
//*Receives signals to pump insulin and pumps for the appropriate periods
//*	-Returns status when requested
//*	-Gives warning when supplies are accessed while critically low
//*	-Allows refilling the reservoir completely (to 1 minute's supply)
//*	 on button press (P1.2)
//*
//*Safety Features:
//*	-Active watchdog to avoid getting stuck in a loop or interrupt
//*	-Button debouncing by sampling when the watchdog is pet
//*	 and requiring 5 matching samples in a row to change state
//*	-Prevents the reservoir from going negative by only pumping
//*	 at most what is left in the reservoir
//*
//*Honor Code: I have neither given or received, nor have I tolerated
//*				others' use of unauthorized aid.
//*				--James Douglas and Kostadin Pendev					
//*********************************************************************************

// Preprocessor Section (includes and defines)

#include <msp430fr6989.h>	//Header file for our board

#define SMCLK   	0x0200 	//Value to set for SMCLK as timer source
#define ACLK   		0x0100 	//Value to set for ACLK as timer source
#define DIV4		0x0080	//Value to set for dividing source by 4
#define UP    		0x0010 	//Value to set for UP counting
#define CLEAR 		0x0004 	//Value of Clock Clear

#define PETDOG		0x5A0C	//Value to Pet Watchdog in Normal Mode
#define ENABLE_PINS 0xFFFE 	// Value to enable pins

#define UART_CLK_SEL 0x0080 // Specifies accurate clock for UART peripheral
#define BR0_FOR_9600 0x34 // Value required to use 9600 baud
#define BR1_FOR_9600 0x00 // Value required to use 9600 baud
#define CLK_MOD 0x4911 // Microcontroller will "clean-up" clock signal

void select_clock_signals(void); // Assigns microcontroller clock signals
void assign_pins_to_uart(void); // P4.2 is for TXD, P4.3 is for RXD
void use_9600_baud(void); // UART operates at 9600 bits/second

// Global variables (reservoir level and remaining 20ms periods)
int level=1500;
int numPers=0;

// Main loop (with setup at start)
void main(void){
	PM5CTL0 = ENABLE_PINS; // Enable pins

	// Set up Output pin (same as red LED)

	P1DIR |= BIT0;
	P1OUT &= ~BIT0;

	P1OUT |= BIT1;
	P1REN |= BIT1;

	// Set up UART
	select_clock_signals(); // Assigns microcontroller clock signals
	assign_pins_to_uart(); // P4.2 is for TXD, P4.3 is for RXD
	use_9600_baud(); // UART operates at 9600 bits/second
	UCA0IE |= 0x0001; // Enable UART RXD interrupt

	// Set up Timers

	TA0CCR0 = 8000; 	 //Period for watchdog petting
	TA0CCTL0 |= CCIE; //Enable watchdog petting interrupt

	TA1CCR0 = 40000; //Period for pump(20ms)
	TA1CCTL0 |= CCIE; //Enable pump on interrupt

	TA1CCR1 = 4000; //Duty Cycle for pump
	TA1CCTL1 |= CCIE; //Enable pump off interrupt

	// Start Timers
	TA0CTL = SMCLK|UP|CLEAR;	//start watchdog timer
	TA1CTL = SMCLK|DIV4|UP|CLEAR;	//set up pump timer

	_BIS_SR(GIE); // Activate all interrupts

	for(;;); // Infinite loop
}

//*********************************************************************************
//* Watchdog Petting Interrupt Service Routine
//*********************************************************************************
#pragma vector=TIMER0_A0_VECTOR
__interrupt void petDog(void){
	
	WDTCTL = 0x5A0C;	//pet watchdog
	
	//button debounce and sample code
	
	//last button press value (1 for not pressed,0 for pressed)
	static int lastPress = BIT1;
	//count of presses in a row opposing the last button press value
	static int count = 0;
	
	//Increment if sample is different than last press, otherwise reset count
	count = ((P1IN ^ lastPress) & BIT1) ? count+1 : 0;
	
	if(count>=5){	//if enough opposing samples have occurred
		count = 0;	//reset opposing count
		lastPress ^= BIT1;//toggle last press
		if(lastPress == 0)level=1500;	//fill reservoir if button pressed
	}
}

//*********************************************************************************
//* Pump On Interrupt Service Routine
//* Called on Timer Rollover
//*********************************************************************************
#pragma vector=TIMER1_A0_VECTOR
__interrupt void pumpOn(void){
	//if we have pumping to do still, turn the pump on
	if(numPers > 0){
		P1OUT |= BIT0;	//set pump high
		numPers--;	//remove 1 from remaining periods to pump
	}
}

//*********************************************************************************
//* Pump Off Interrupt Service Routine
//* Called on Hitting Duty Cycle Count
//*********************************************************************************
#pragma vector=TIMER1_A1_VECTOR
__interrupt void pumpOff(void){
	P1OUT &= ~BIT0;		//set pump low
	TA1CCTL1 &= ~CCIFG; //clear interrupt flag
}

//*********************************************************************************
//* UART RX Interrupt *
//*Looks at received pump code, acts accordingly*
//*********************************************************************************
#pragma vector=USCI_A0_VECTOR
__interrupt void UART_ISR(void) {
	//get sent message
	int message = UCA0RXBUF;
	
	//if the code was to pump, pump
	if(message >= 0x01 && message <= 0x7D){
		//if there is under 5 seconds insulin left, send an alert
		if(level <= 0x7D) UCA0TXBUF=0xFF;
		//get correct dosage (minimum of passed value and reservoir level)
		int dosage = (message <= level) ? message : level;
		//add pump periods and remove used insulin from reservoir
		numPers += 2*dosage;
		level -= dosage;
	}
	else if (message == 0xFF){ // if the code was to send status, send status
		//if pumping is done
		if(numPers==0){
			//get appropriate message based on reservoir levels
			if(level<125){
				UCA0TXBUF=0x03;
			}
			else if(level<625){
				UCA0TXBUF=0x02;
			}
			else{
				UCA0TXBUF=0x01;
			}
		}
		else{
			//get appropriate message based on reservoir levels
			if(level<125){
				UCA0TXBUF=0x8A;
			}
			else if(level<625){
				UCA0TXBUF=0x89;
			}
			else{
				UCA0TXBUF=0x88;
			}
		}
	}
	UCA0IFG = UCA0IFG & (~UCRXIFG); // Clear RX Interrupt FlaG
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
	P4SEL1 = 0x00; // 0000 0000
	P4SEL0 = BIT3 | BIT2; // 0000 1100
	// ^^
// ||
// |+---- 01 assigns P4.2 to UART Transmit (TXD)
// |
// +----- 01 assigns P4.3 to UART Receive (RXD)
}

//*********************************************************************************
//* Specify UART Baud Rate *
//*********************************************************************************
void use_9600_baud(void) {
	UCA0CTLW0 = UCSWRST; // Put UART into SoftWare ReSeT
	UCA0CTLW0 = UCA0CTLW0 | UART_CLK_SEL; // Specifies clock sourse for UART
	UCA0BR0 = BR0_FOR_9600; // Specifies bit rate (baud) of 9600
	UCA0BR1 = BR1_FOR_9600; // Specifies bit rate (baud) of 9600
	UCA0MCTLW = CLK_MOD; // "Cleans" clock signal
	UCA0CTLW0 = UCA0CTLW0 & (~UCSWRST); // Takes UART out of SoftWare ReSeT
}

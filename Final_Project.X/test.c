

#include <xc.h>
#include <stdio.h>
#pragma config OSC = IRCIO, MCLRE = OFF, LVP=OFF
#define _XTAL_FREQ 8000000
#include "LCDScreen.h"
#define LCD_RS LATAbits.LATA6 
#define LCD_E LATCbits.LATC0

void Init(void) {
    LATC = 0;
    TRISCbits.RC6 = 1;
    TRISCbits.RC7 = 1;
    OSCCON = 0b11110010;
    while (!OSCCONbits.IOFS);
    SPBRG = 204;
    SPBRGH = 0;
    BAUDCONbits.BRG16 = 1;
    TXSTAbits.BRGH = 1;
    RCSTAbits.CREN = 1;
    RCSTAbits.SPEN = 1;
    TXSTAbits.TXEN = 1;
    //enable transmitter, other settings default
}


void IR_Init() {
    LATA = 0; //Tensure all pins have same value initially
    TRISA = 0b00001100; //Turn on CAP1 (RA2) and CAP2 (RA3) Pins.
    CAP1CON = 0b01000110; //Time base enabled, measure PWM falling to rising 
    CAP2CON = 0b01000110;
    T5CON = 0b00001001; //Timer 5 prescaler 1:2, Timer on (overflow 65.536ms - max PWM is 50ms)
    DFLTCON = 0b00011000; //Noise filter for CAP1 and CAP2 
}
void IRScreen(int LeftIR, int RightIR) { //write IR values on screen
    char buf[16];
    SendLCD(0x20, 1);
    __delay_us(50);
    SetLine(2);
    sprintf(buf, " L:%u R:%u", LeftIR, RightIR);
    LCD_String(buf);
//    SetLine(2);
//    sprintf(buf, " Right: %u", RightIR);
//    LCD_String(buf);
//        char buf[16];
//        SendLCD(0x20,1); 
//        SetLine(1);
//        sprintf(buf," Left: %u",LeftIR);        
//        LCD_String(buf);
//        SetLine(2);
//        sprintf(buf," Right: %u",RightIR);        
//        LCD_String(buf);
}

void FindIR(unsigned int *L_IR, unsigned int *R_IR) {
    *L_IR = ((CAP1BUFH << 8) | CAP1BUFL); //bit shift high val and OR it with low for total reading
    *R_IR = ((CAP2BUFH << 8) | CAP2BUFL); //values between 0 and 50000
}
        unsigned char duf;


void main(void) {
    ANSEL0 = 0b00000000; // Set pin AN3 to analog input
    ANSEL1 = 0; // Set rest of AN pins as Digital I/O

    ADCON0 = 0b00001101; // Signle shot, ADC port channel 3 (AN3), Enable ADC
    ADCON1 = 0b00000000; // Use Internal Voltage Reference
    // (Vdd and Vss)
    ADCON2 = 0b10101011; // Right justify result and timing settings

    OSCCON = 0b11110010;
    while (!OSCCONbits.IOFS);

    //Component Initialisation
  
    LCD_Init(); //Initialize the LCD
 
    IR_Init(); //Initialise IR Readers

    //    Timer5_Init();
//    SendLCD(0b00011000, 0);

  // Call Variables
    unsigned int RightIR;
    unsigned int LeftIR;
   

           
    while(duf == 0){
        FindIR(&LeftIR, &RightIR);
        RightIR = RightIR/100;
        LeftIR = LeftIR/100;
        IRScreen(LeftIR,RightIR);

    }
    
    
}
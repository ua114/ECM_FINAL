/* 
 * File:   main.c
 * Author: rh2415
 *
 * Created on 22 November 2018, 16:09
 */
#include<string.h>
#include <pic18f4331.h>
#include <xc.h>

#include <stdio.h>
#include "MotorControl.h"

#pragma config OSC = IRCIO // internal oscillator
#define _XTAL_FREQ 8000000 
#define LCDP8 LATCbits.LATC0
#define LCDP8 LATCbits.LATC0 // toggle pin
#define LCD_RS LATAbits.LATA6 // RS pin
#define LCD_B7 LATDbits.LATD1 // Pin 16 LCD
#define LCD_B6 LATDbits.LATD0 // Pin 15 LCD
#define LCD_B5 LATCbits.LATC2 // Pin 14 LCD
#define LCD_B4 LATCbits.LATC1// Pin 13 LCD

 struct DC_motor motorL, motorR; //declare two DC_motor structures
 
void IR_INIT (void)
{
    LATA=0;  
    TRISA=0b00001100; //Turn on CAP1 CAP2 Pins.
    CAP1CON=0b01000110;//bit 0-3 measures falling to riding 
   CAP2CON=0b01000110;
     
    T5CON=0b00001001;
    DFLTCON = 0b00011000; //turn ON oise filter for CAP1 and CAP2 
}

void storeIR(unsigned int *IR_R, unsigned int *IR_L) {
    *IR_L = ((CAP1BUFH << 8) | CAP1BUFL); //seperate 16 bit number to low and high
    *IR_R = ((CAP2BUFH << 8) | CAP2BUFL); //values between 0 and 50000
}
unsigned int counter=0;
unsigned int arrayCount[];
void fwdCount()
{
     arrayCount[counter]=0;
}

void leftCount()
{
    arrayCount[counter]=1;
}

void rightCount()
{
     arrayCount[counter]=2;
}


void TrackIR(unsigned int LeftIR, unsigned int RightIR) {
    
   unsigned int limit = 25;
//    FindIR(&LeftIR, &RightIR);
//    RightIR = RightIR / 100;
//    LeftIR = LeftIR / 100;
//    IRScreen(LeftIR,RightIR);
    if ((LeftIR - RightIR)>limit ) {
           turnLeft(&motorL, &motorR);
             if(counter==0){
           leftCount();
        } else  {
            counter++;
            leftCount();
            
        }
        
    }
    else if ((RightIR - LeftIR)>limit) {
            turnRight(&motorL, &motorR);
            if(counter==0){ //if this is the first step, store it in the first array 
           rightCount();
        } else        {//if not the first step, increment by 1 then store
            counter++;
            rightCount();
        }
    }
    else if (abs(RightIR-LeftIR)<=limit)
    {
       fullSpeedAhead(&motorL, &motorR);
        if(counter==0){
            fwdCount();
        } else        {
            counter++;
             fwdCount();
        }
    }
     
   
}

void E_TOG(void){
    LCDP8 = 1;
__delay_us(5); // 5us delay ? remember to define _XTAL_FREQ
    LCDP8 = 0;
}
//function to send four bits to the LCD
void LCDout(unsigned char number){
//set data pins using the four bits from number
 //toggle the enable bit to send data

    if (number & 0b1000){LCD_B7=0b0001;} else{LCD_B7=0;}
    if (number & 0b0100){LCD_B6=0b0001;} else{LCD_B6=0;}
    if (number & 0b0010){LCD_B5=0b0001;} else{LCD_B5=0;}
    if (number & 0b0001){LCD_B4=0b0001;} else{LCD_B4=0;}
    E_TOG();
    __delay_us(5); // 5us delay
}
//function to send data/commands over a 4bit interface
void SendLCD(unsigned char Byte, char type)
{
    // set RS pin whether it is a Command (0) or Data/Char (1)
    // using type as the argument
    LCD_RS =type;
    // send high bits of Byte using LCDout function
    LCDout((Byte&0b11110000)>>4); // Store the 4 most significant bits
    //
     // send low bits of Byte using LCDout function
     LCDout(Byte&0b00001111); // Store the 4 least significant bits
     __delay_us(50);
 
}

void LCD_Init(void){
    // set initial LAT output values (they start up in a random state)
    LATC=0;
    LATA=0;
    LATD=0;
    // set LCD pins as output (TRIS registers)
    TRISC= 0;
    TRISD= 0;
   // TRISA= 0;

 // Initialisation sequence code - see the data sheet

 //delay 15mS
     __delay_us(15);
 //send 0b0011 using LCDout
     LCDout(0b0011);
 //delay 5ms
     __delay_ms(5);
 //send 0b0011 using LCDout
     LCDout(0b0011);
 //delay 200us
     __delay_us(200);
 //send 0b0011 using LCDout
     LCDout(0b0011);
 //delay 50us
     __delay_us(50);
 //send 0b0010 using LCDout set to four bit mode
     LCDout(0b0010);
 // now use SendLCD to send whole bytes ? send function set, clear
    SendLCD(0b00101000,0); //function set
    __delay_us(50);
    SendLCD(0b00001000,0); //Display off
    __delay_us(50);
    SendLCD(0b00000001,0); //clear screen
    __delay_ms(5);
    SendLCD(0b00000110,0); //set entry mode
    __delay_us(50);
     SendLCD (0b00001110, 0); //Display on, Cursor on, blinking off
    __delay_us(50);
 // screen, set entry mode, display on etc to finish initialisation
}

//function to put cursor to start of line
void SetLine (char line){
//Send 0x80 to set line to 1 (0x00 ddram address)
    if (line==1){
        SendLCD(0x80,0);   // set cursor to begining of line 1
    }
    //Send 0xC0 to set line to 2 (0x40 ddram address)
    if (line==2){
        SendLCD(0xC0,0);
    }
    __delay_us(50); // 50us delay}
}


void LCD_String(char *string){
    //While the data pointed to isn?t a 0x00 do below
   while(*string != 0){
    //Send out the current byte pointed to
    // and increment the pointer
   SendLCD(*string++,1);
    __delay_us(50); //so we can see each character
    //being printed in turn (remove delay if you want
    //your message to appear almost instantly)
    }
}

void initPWM(void) // function to setup PWM
{
    ////WE LEFT OFF BY CHANGING THE pwm, NEED TO CHANGE THE PWMCON0 
    
TRISBbits.RB0=0;

    TRISBbits.RB1 = 0;
    TRISBbits.RB2 = 0;
    TRISBbits.RB3 = 0;
PTCON0 = 0b00001100; // free running mode, 1:64 prescaler = 32 us
PTCON1 = 0b10000000; // enable PWM timer
PWMCON0 = 0b01101111; // PWM0/1 enabled, all independent mode
PWMCON1 = 0x00; // special features, all 0 (default)
PTPERL = 0b11000111; // base PWM period low byte
PTPERH = 0b00000000; // base PWM period high byte
//note: the two LSBs in PDC0L have a different function;
//this mean to you need to shift your on period to the left <<
//by 2 bits
//50% is 199/2
int period=99;
    //assigning on period values to PWM1
    PDC0L = (period<<2); // PDC0L bits 7:2 are on period bits 5:0
    PDC0H = (0b00000000>>6); // PDC0H bits 5:0 are on period bits 11:6

//assigning on period values to PWM3
    PDC1L = (period<<2); // PDC0L bits 7:2 are on period bits 5:0
    PDC1H = (0b00000000>>6); // PDC0H bits 5:0 are on period bits 11:6
}

void Motor_Init()
{
int PWMcycle=199;
    //init L motor
    
motorL.power=0; //zero power to start
motorL.direction=1; //set default motor direction
motorL.dutyLowByte=(unsigned char *)(&PDC0L); //store address of PWM duty low byte
motorL.dutyHighByte=(unsigned char *)(&PDC0H); //store address of PWM duty high byte
motorL.dir_pin=0; //pin RB0/PWM0 controls direction
motorL.PWMperiod=PWMcycle; //store PWMperiod for motor


//same for motorR but different PWM registers and direction pin 

motorR.power=0; //zero power to start
motorR.direction=1; //set default motor direction
motorR.dutyLowByte=(unsigned char *)(&PDC1L); //store address of PWM duty low byte
motorR.dutyHighByte=(unsigned char *)(&PDC1H); //store address of PWM duty high byte
motorR.dir_pin=2; //pin RB0/PWM0 controls direction
motorR.PWMperiod=PWMcycle;
}

void main(void)
{
    LATC=0;
    LATD=0;
    TRISC=0;
    TRISD=0;
    unsigned int RightIR;
    unsigned int LeftIR;
    ANSEL0 = 0b00000000; // Set pin AN3 to analog input
    ANSEL1 = 0; // Set rest of AN pins as Digital I/O
    /* Init ADC */
    ADCON0 = 0b00001101; // Single shot, ADC port channel 3 (AN3), Enable ADC
    ADCON1 = 0b00000000; // Use Internal Voltage Reference
    // (Vdd and Vss)
    ADCON2 = 0b10101011; // Right justify result and timing settings
     char buf[16];
     char buff[16];
     OSCCON = 0x72; //8MHz clock}
    while (!OSCCONbits.IOFS); //Wait for OSC to become stable
   
    IR_INIT();
    LCD_Init(); //Initialize the LCD
    initPWM();
    Motor_Init();
    
    SendLCD(0b00000001,0); //clear screen
    __delay_ms(50);
    
    while (1)
    {
        
     //Find and store IR values
    storeIR(&LeftIR,&RightIR);
    LeftIR=LeftIR/10;
    RightIR=RightIR/10;
    TrackIR(LeftIR, RightIR);
    
    //Print IR VAlues to Screen
      SetLine(1);
    sprintf(buf,"L:%u",LeftIR);
     LCD_String(buf);

      SetLine(2);
       sprintf(buff,"R:%u", RightIR);
    SetLine(2);

     LCD_String(buff);
         
    __delay_ms(500);
    SendLCD(0b00000001,0); //clear screen
   
    }
}

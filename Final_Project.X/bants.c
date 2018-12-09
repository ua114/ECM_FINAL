#include <xc.h>
#include <stdio.h>
#pragma config OSC = IRCIO, MCLRE = OFF, LVP=OFF
#define _XTAL_FREQ 8000000
#include "LCDScreen.h"
#include "DCMotor.h"
#define LCD_RS LATAbits.LATA6 
#define LCD_E LATCbits.LATC0
//#define LCD_DB4 PORTCbits.RC1
//#define LCD_DB5 PORTCbits.RC2
//#define LCD_DB6 PORTDbits.RD0
//#define LCD_DB7 PORTDbits.RD1
//LCD Register Select PIN
//LCD_RS =1;
//// pin RA6 is now high
//LCD_RS=0;
// pin RA6 is now low
//don?t forget to put function prototypes at the start of your program
// precise delays can be produced using the __delay_us() and __delay_ms() 
// macro functions. For these to work you must write the line
//#define _XTAL_FREQ 8000000 //i.e. for an 8MHz clock frequency
// near the top of your main.c file so the functions can calculate how 
// long to make the delay for different clock frequencies.
//function to toggle enable bit on then off

struct DC_motor motorL, motorR;
char RFID[11];
unsigned char duf;

void Init(void) 
{
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

unsigned char getCharSerial(void) {
    while (!PIR1bits.RCIF);
    return RCREG;
    //return byte in RCREG
}

void Interrupt_Init(void){
    INTCONbits.GIEH = 1; // Global interrupt Enable bit
    INTCONbits.GIEL = 1; // Peripheral interrupt enable bit 
    PIE1bits.RCIE = 1;
    RCONbits.IPEN = 1; //enable priority
//    PIE3bits.TMR5IE = 1;
   
}

void RFID_Init(void) {
    LATC = 0;
    TRISDbits.RD2 = 0;
    TRISCbits.RC6 = 1;
    TRISCbits.RC7 = 1;
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
    *L_IR = ((CAP1BUFH << 8) | CAP1BUFL); //seperate 16 bit number to low and high
    *R_IR = ((CAP2BUFH << 8) | CAP2BUFL);//values between 0 and 50000
}
        unsigned char duf;
void high_priority interrupt InterruptHandlerHigh() {

    if (PIR1bits.RCIF == 1) {
        
        SetLine(1);
        int charcount=0;
        duf = getCharSerial();
        if (duf == 0x02) {
            while (duf != 0x03) {
                duf = getCharSerial();
                if (charcount <12){
                    SendLCD(duf, 1);
                    LATDbits.LATD2 = 1;
                    charcount++;
                }
                
            }
        }
    }
}

//void low_priority interrupt InterruptHandlerLow() {
//    if (PIR3bits.TMR5IF == 1) {
//        ClearScreen();
//        unsigned int RightIR;
//        unsigned int LeftIR;
//        FindIR(&LeftIR, &RightIR);
//        RightIR = RightIR/100;
//        LeftIR = LeftIR/100;
//        IRScreen(LeftIR,RightIR);
//    }
//    PIR3bits.TMR5IF = 0;
//}

void Motor_Init(){ 
    int PWMcycle = 199; //Cycle length for 10kHz PWM 
    initPWM();
    
    // initialise left motor
    motorL.power=0; //zero power to start
    motorL.direction=1;//set default motor direction
    motorL.dutyLowByte=(unsigned char*)(&PDC0L);//store address of PWM duty low byte
    motorL.dutyHighByte=(unsigned char*)(&PDC0H);//store address of PWM duty high byte
    motorL.dir_pin=0;//pin RB0/PWM0 controls direction
    motorL.PWMperiod=PWMcycle;//store PWMperiod for motor
    
    // initialise right motor
    motorR.power=0; //zero power to start
    motorR.direction=1;//set default motor direction
    motorR.dutyLowByte=(unsigned char*)(&PDC1L);//store address of PWM duty low byte
    motorR.dutyHighByte=(unsigned char*)(&PDC1H);//store address of PWM duty high byte
    motorR.dir_pin=2;//pin RB0/PWM0 controls direction
    motorR.PWMperiod=PWMcycle;//store PWMperiod for motor
}

char CharConversion(unsigned char number){
    if(number>64 && number<71){
        number-=55;
    }else if(number>47 && number<58){
        number-=48;
    }
    return number;
}


void TrackIR(int LeftIR, int RightIR) {
    int limit = 150;
//    FindIR(&LeftIR, &RightIR);
//    RightIR = RightIR / 100;
//    LeftIR = LeftIR / 100;
//    IRScreen(LeftIR,RightIR);
    if (LeftIR - RightIR > limit) {
        moveMotor(&motorL, &motorR,50,50,1,0);
    }
    else if (RightIR - LeftIR > limit) {
        moveMotor(&motorL, &motorR,50,50,0,1);
//    } else if (RightIR-LeftIR>0){
//        moveMotor(&motorL, &motorR,60,60+((RightIR-LeftIR)/10),0,1);
    }
    else {
        moveMotor(&motorL, &motorR,50,50,1,1);
    }
}
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
    Interrupt_Init(); // Initialise Interrupt Procedure
    LCD_Init(); //Initialize the LCD
    RFID_Init(); // Initialise RFID Reader
    Motor_Init(); // Initialise Motors
    IR_Init(); //Initialise IR Readers

    //    Timer5_Init();
//    SendLCD(0b00011000, 0);

  // Call Variables
    unsigned int RightIR;
    unsigned int LeftIR;
    char checksum=0;
    char checksumresult=0;
    char StepCounter=0;
    volatile signed char LeftPowArray [100];
    volatile signed char RightPowArray [100];
    volatile signed char LeftDirArray [2];
    volatile signed char RightDirArray [2];
    
    TRISCbits.RC3 = 1;
    stop(&motorL, &motorR);
           
    while(PORTCbits.RC3==0); // Turn on when button pressed

//    while(PORTCbits.RC3==0);
    while(duf == 0){
        FindIR(&LeftIR, &RightIR);
        RightIR = RightIR/100;
        LeftIR = LeftIR/100;
        IRScreen(LeftIR,RightIR);
////        FindIR(&LeftIR, &RightIR);
        TrackIR(LeftIR, RightIR);
        LeftPowArray[StepCounter]=motorL.power;
        RightPowArray[StepCounter]=motorR.power;
        LeftDirArray[StepCounter]=motorL.direction;
        RightDirArray[StepCounter]=motorR.direction;
        StepCounter++;
        delay_halfs(1);
    }
    while(duf!=0){
    stop(&motorL, &motorR);
    unsigned char checksum = 0; //0 is identity for  check sum
    unsigned char checksumresult = 0;
    for (char i =0;i<6;i++){ //converts all ASCII bits of data to decimal
        
        
            RFID[(2*i)] = CharConversion(RFID[(2*i)]);
        RFID[(2*i)+1] = CharConversion(RFID[(2*i)+1]);
        if (i<5){
            checksum = checksum ^ ( (RFID[(2*i)]<<4) + RFID[(2*i)+1]); //XOR 5 data bytes 
        }
        else{
            checksumresult = ( (RFID[(2*i)]<<4) + RFID[(2*i)+1] ); //only for last 2 bytes
        }
    }
    if (checksum==checksumresult){
        SetLine(2);
        SendLCD(0x20,1);
        LCD_String("accepted");
    }else{
        SetLine(2);
        SendLCD(0x20,1);
        LCD_String("NO");
    }
    delay_s(1);
    while(StepCounter>0){
        StepCounter--;
        if (LeftDirArray[StepCounter]==0){
            LeftDirArray[StepCounter]=1;
        }else{
            LeftDirArray[StepCounter]=0;
        }
        if(RightDirArray[StepCounter]==0){
            RightDirArray[StepCounter]=1;
        }else{
            RightDirArray[StepCounter]=0;
        }
        moveMotor(&motorL, &motorR,LeftPowArray[StepCounter],RightPowArray[StepCounter],LeftDirArray[StepCounter],RightDirArray[StepCounter]);
        delay_halfs(1);
    }
    while(StepCounter==0){
        stop(&motorL,&motorR);
        delay_s(1);
    }
    }
}

#include <xc.h>
#include <stdio.h>
#include "LCDScreen.h"
#include "MotorControl.h"
#pragma config OSC = IRCIO, MCLRE = OFF, LVP=OFF
#define _XTAL_FREQ 8000000
unsigned int counter=0;
unsigned int arrayCount[50];
struct DC_motor motorL, motorR;

void IR_Init() {
    LATA = 0; //Tensure all pins have same value initially
    TRISA = 0b00001100; //Turn on CAP1 (RA2) and CAP2 (RA3) Pins.
    CAP1CON = 0b01000110; //Time base enabled, measure PWM falling to rising 
    CAP2CON = 0b01000110;
    T5CON = 0b00001001; //Timer 5 prescaler 1:2, Timer on (overflow 65.536ms - max PWM is 50ms)
    DFLTCON = 0b00011000; //Noise filter for CAP1 and CAP2 
}

void FindIR(unsigned int *L_IR, unsigned int *R_IR) {
    *L_IR = ((CAP1BUFH << 8) | CAP1BUFL); //bit shift high val and OR it with low for total reading
    *R_IR = ((CAP2BUFH << 8) | CAP2BUFL); //values between 0 and 50000
}
 

void IRScreen(int LeftIR,int RightIR){  //write IR values on screen
        char buf[16];
         SendLCD(0b00000001,0); //clear screen              
        sprintf(buf,"L: %u",LeftIR);        
        LCD_String(buf);
        SetLine(2);
        sprintf(buf,"R: %u",RightIR);        
        LCD_String(buf);
}

void IRScreen2( int counters){  //write IR values on screen
        char buf[16];
         SendLCD(0b00000001,0); //clear screen              
        sprintf(buf,counters);        
        LCD_String(buf);
        
}

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

void TrackIR(int LeftIR, int RightIR) {
    int limit=10; //################## fine tune 
    
     if ((LeftIR - RightIR)>limit) {
    if(counter==0){
    turnLeft(&motorL, &motorR);
         leftCount();
         counter++;
         __delay_ms(400);
      stop(&motorL, &motorR);
    } else {
    turnLeft(&motorL, &motorR);
        leftCount();
        counter++;
        __delay_ms(400);
      stop(&motorL, &motorR);
    }     
    }
    else if ((RightIR -LeftIR) >limit) {

    if(counter==0){
        turnRight(&motorL, &motorR);
        rightCount();
        counter++;
        __delay_ms(400);
      stop(&motorL, &motorR);
} else {
   
        turnRight(&motorL, &motorR);
        rightCount();
        counter++;
        __delay_ms(400);
      stop(&motorL, &motorR);
}    
}
    else {
        if(RightIR>200 && LeftIR>200)
            {
            if(counter==0){
            forward(&motorL, &motorR);
            fwdCount();
            counter++;
            __delay_ms(400);
      stop(&motorL, &motorR);
            } else {

            forward(&motorL, &motorR);
            fwdCount();
            counter++;
            __delay_ms(400);
      stop(&motorL, &motorR);
            }    
        }
        else if(RightIR<100 && LeftIR<100) {
            
            if(counter==0){
            turnLeft(&motorL, &motorR);
            leftCount();
            counter++;
            __delay_ms(400);
      stop(&motorL, &motorR);
            } else {

            turnLeft(&motorL, &motorR);
            leftCount();
            counter++;
            __delay_ms(400);
      stop(&motorL, &motorR);
            }    
        }
        
        
    }

     
       stop(&motorL, &motorR);
}



void Init(void) {
    LATC = 0;
    TRISCbits.RC7 = 1;
    OSCCON = 0b11110010;
    while (!OSCCONbits.IOFS);
    SPBRG = 205;
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
char read=0;
void interrupt InterruptHandlerHigh (void)
{
    if(PIR1bits.RCIF==1)
    {
    
    unsigned char readIn=0;
    int read_counter=0;
    readIn = getCharSerial();
         
     if (readIn == 0x02) {
        //    SendLCD(0b00000001,0); //clear screen 
       //  __delay_ms(205);
            while (readIn != 0x03) {
                readIn = getCharSerial();
                if (read_counter <12){
                    SendLCD(readIn, 1);
                    read_counter++;
                }
            }
        }
     stop(&motorL, &motorR);
     read=1;
    }
}
void InterruptINIT()
{
    RCONbits.IPEN=1;// Enable interrupt priority
    INTCONbits.GIEH = 1; // Global interrupt enable bit
    INTCONbits.GIEL = 1; //enable peripheral interrupts
    PIE1bits.RCIE = 1; //Enable EUSART interrupt
}
void main(void) {
     ANSEL0=0; //Override start up analogue mode to digital instead
    ANSEL1=0;          
    OSCCON = 0x72; //Set up timer
    while(!OSCCONbits.IOFS);   //Wait till timer set up
    
    unsigned int RightIR;
    unsigned int LeftIR; 
    unsigned char buf[16]; //for writing strings to lcd
  
    // initalise 
    LCD_Init();
    InterruptINIT();
    IR_Init(); 
    Motor_Init(); // Initialise Motors
    Init();
    stop(&motorL, &motorR);
    TRISCbits.RC3 = 1;  //Button bit
    char i;
    sprintf(buf,"Push Button");
    LCD_String(buf);
    SetLine(2);
    sprintf(buf,"can we leave now?!");;
    LCD_String(buf);
    while(PORTCbits.RC3==0);   //Button press detection
     
    __delay_ms(100);
    SendLCD(0b00000001,0); //clear screen 
    read=0;
    char fin=0;
    while (1){
        if(read ==0){
            FindIR(&LeftIR, &RightIR);
            RightIR = RightIR/100;
            LeftIR = LeftIR/100;
            //IRScreen(LeftIR,RightIR);
            TrackIR(LeftIR, RightIR);
           // IRScreen2(counter);
        }
        
        if (read==1 && fin==0)
        {
              for (i=0; i<counter; i++)
    {
    if(arrayCount[counter-i]==0)
    {
    back(&motorL, &motorR);
    } else if(arrayCount[counter-i]==1)
    {
    turnRight(&motorL, &motorR);
    } else if(arrayCount[counter-i]==2)
    {
    turnLeft(&motorL, &motorR);
    }
__delay_ms(400); // delay for a second 
  stop(&motorL, &motorR);
    }
              fin=1;
     
            }
    }
}

        


    //this is the one


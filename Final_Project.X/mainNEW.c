#include <xc.h>
#include <stdio.h>
#include "LCDScreen.h"
#include "MotorControl.h"
#pragma config OSC = IRCIO, MCLRE = OFF, LVP=OFF
#define _XTAL_FREQ 8000000

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
        SetLine(1);
         sprintf(buf,"Left: %u",LeftIR);        
        LCD_String(buf);
        SetLine(2);
        sprintf(buf,"Right: %u",RightIR);        
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


void TrackIR(unsigned int LeftIR, unsigned int RightIR) {
   unsigned int limit=25;
   
if ((LeftIR - RightIR)>limit) {
        turnLeft(&motorL, &motorR);
      
        }
else if ((RightIR - LeftIR)>limit) {
        turnRight(&motorL, &motorR);
        
        }
else if (abs(RightIR-LeftIR)<=limit && RightIR>4000 && LeftIR>4000)
    {
       fullSpeedAhead(&motorL, &motorR);
      
      
    }
    
//    if(LeftIR >3 - RightIR > 500)
//    {
//            if (LeftIR > RightIR) {
//                moveMotor(&motorL, &motorR, 50,50,0,1);//turn left
//
//                }
//            else if (LeftIR < RightIR) {
//                moveMotor(&motorL, &motorR, 50,50,1,0);
//                }
//    }
//    
//    if () {
//        
//        if (LeftIR <2000 && RightIR<2000)
//        {
//           moveMotor(&motorL, &motorR, 50,50,1,0);  //lostt
//        }
//         else 
//        {
//         moveMotor(&motorL, &motorR, 70,70,1,1);
//        }
//    }
    
}

void main(void) {
     ANSEL0=0; //Override start up analogue mode to digital instead
    ANSEL1=0;          
    OSCCON = 0x72; //Set up timer
    while(!OSCCONbits.IOFS);   //Wait till timer set up
    unsigned int diff;
    unsigned int RightIR;
    unsigned int LeftIR; 
    unsigned char buf[16]; //for writing strings to lcd
    
    // initalise 
    LCD_Init();
    IR_Init(); 
    Motor_Init(); // Initialise Motors
    stop(&motorL, &motorR);
    TRISCbits.RC3 = 1;  //Button bit
    
    sprintf(buf,"Push Button");
    LCD_String(buf);
    SetLine(2);
    sprintf(buf,"to start :)");
    LCD_String(buf);
    while(PORTCbits.RC3==0);   //Button press detection
    
    __delay_ms(100);
  
    while (1)
    {
   
    FindIR(&LeftIR, &RightIR);
    RightIR = RightIR/10;
    LeftIR = LeftIR/10;
    IRScreen(LeftIR,RightIR);
    
    LCD_String(buf);
    TrackIR(LeftIR, RightIR);
    //moveMotor(&motorL, &motorR,40,40,1,0);
    __delay_ms(1000);
    }
}
#include <xc.h>
#pragma config OSC = IRCIO // internal oscillator
#define _XTAL_FREQ 8000000 
int PWR=199;
void initPWM(void) // function to setup PWM
{
    ////WE LEFT OFF BY CHANGING THE pwm, NEED TO CHANGE THE PWMCON0 
    
TRISBbits.RB0=0;
TRISBbits.RB1 = 0;
TRISBbits.RB2 = 0;
TRISBbits.RB3 = 0;
PTCON0 = 0b00001100; // free running mode, 1:64 prescaler = 32 us
PTCON1 = 0b10000000; // enable PWM timer~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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


void delay_s(char seconds) // function to delay in seconds
//__delay_ms() is limited to a maximum delay of 89ms with an 8Mhz
//clock so you need to write a function to make longer delays
{
    for(char i=0; i<(seconds*20); i++)
    {
       __delay_ms(50);
    }
}


struct DC_motor {
 char power; //motor power, out of 100
 char direction; //motor direction, forward(1), reverse(0)
 unsigned char *dutyLowByte; //PWM duty low byte address
 unsigned char *dutyHighByte; //PWM duty high byte address
 char dir_pin; // pin that controls direction on PORTB
 int PWMperiod; //base period of PWM cycle
}; //struct motor


// function to set PWM from the values in the motor structure
void setMotorPWM(struct DC_motor *m)
{
 int PWMduty; //tmp variable to store PWM duty cycle
 if (m->direction){ //if forward
 // low time increases with power
 PWMduty=m->PWMperiod - ((int)(m->power)*(m->PWMperiod))/100;
 }
 else { //if reverse
 // high time increases with power
 PWMduty=((int)(m->power)*(m->PWMperiod))/100;//######################################
 }
 PWMduty = (PWMduty << 2); // two LSBs are reserved for other things
 *(m->dutyLowByte) = PWMduty & 0xFF; //set low duty cycle byte
 *(m->dutyHighByte) = (PWMduty >> 8) & 0x3F; //set high duty cycle byte

 if (m->direction){ // if direction is high,
 LATB=LATB | (1<<(m->dir_pin)); // set dir_pin bit in LATB to high
 } else { // if direction is low,
 LATB=LATB & (~(1<<(m->dir_pin))); // set dir_pin bit in LATB to low
 }
}

/*void setMotorPWM(int speed, char direction, char motor)
{
      int PTMR;
    if (motor == 2) {//right motor
        LATBbits.LATB0 = direction;
        if (direction==0){//forward so basic formula
        PTMR = (speed*20)- 1;
        //simplified version for the PTMR formula in the notes
        }
        if (direction==1){//backward so need to 10-speed for correct speed
            PTMR = ((10-speed)*20)- 1;
            
        }
        PDC0L = ( PTMR) << 2;
        PDC0H = ( PTMR) >> 6;
    }

    if (motor == 1) {//left motor
        LATBbits.LATB2 = direction;
        if (direction==0){//forward so basic formula
        PTMR = (speed*20)- 1;
        //simplified version for the PTMR formula in the notes
        }
        if ( direction==1){//backward so need to 10-speed for correct speed
            PTMR = ((10-speed)*20)- 1;
            
        }
          PDC1L = ( PTMR) << 2;
        PDC1H = (PTMR) >> 6;
    }
}*/

void setMotorFullSpeed(struct DC_motor *m)
{
 for (m->power; (m->power)<=100; (m->power)++){ //increase motor power until 100
 setMotorPWM(m); //pass pointer to setMotorSpeed function (not &m here)
 __delay_ms(5); //delay of 5 ms (500 ms from 0 to 100 full power)
 }
}

// high level functions to control left and right motor
// use setMotorPWM to perform low level update
// bring the robot base to a stop
    struct DC_motor motorL, motorR; //declare two DC_motor structures
    
void stop(struct DC_motor *m_L, struct DC_motor *m_R) {
    motorL.direction = 1;
    motorL.power = 0;
setMotorPWM(m_L);
    motorR.direction = 1;
    motorR.power = 0;
setMotorPWM(m_R);
}
// turn robot base left on the spot
void turnLeft(struct DC_motor *m_L, struct DC_motor *m_R,char powerL, char powerR)
{
    motorL.direction=0;
    motorL.power=powerL;
    setMotorPWM(m_L);
    motorR.direction=1;
    motorR.power=powerR;
    setMotorPWM(m_R);
}
// turn robot base right on the spot
void turnRight(struct DC_motor *m_L, struct DC_motor *m_R,char powerL, char powerR)
{
motorL.direction=1;
    motorL.power=powerL;
    setMotorPWM(m_L);
    motorR.direction=0;
    motorR.power=powerR;
    setMotorPWM(m_R);
}
// both motors forward and up to full power
void fullSpeedAhead(struct DC_motor *m_L, struct DC_motor *m_R, char powerL, char powerR)
{
motorL.direction=1;
    motorL.power=powerL;
    setMotorPWM(m_L);
    motorR.direction=1;
    motorR.power=powerR;
    setMotorPWM(m_R);
}

void main (void)
{
    LATB=0;
    LATC = 0; //set the output data latch levels to 0 on all pins
    LATD = 0;
    TRISC = 0; //set the data direction registers to output on all pins
    TRISD = 0;
    TRISA = 0b00001000; // Select pin RA3 as input
    

 OSCCON = 0x72; //8MHz clock}
    while (!OSCCONbits.IOFS); //Wait for OSC to become stable
    
    initPWM(); //setup PWM registers

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

    while (1) {

        fullSpeedAhead(&motorL, &motorR);
        delay_s(5);

        turnLeft(&motorL, &motorR);
        delay_s(5);
        
        turnRight(&motorL, &motorR);
        delay_s(5);

        stop(&motorL, &motorR);
        delay_s(5);
        
    }
 
}
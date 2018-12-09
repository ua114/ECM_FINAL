#include <xc.h>
#include "MotorControl.h"
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

void setMotorFullSpeed(struct DC_motor *m)
{
 for (m->power; (m->power)<=100; (m->power)++){ //increase motor power until 100
 setMotorPWM(m); //pass pointer to setMotorSpeed function (not &m here)
 __delay_ms(5); //delay of 5 ms (500 ms from 0 to 100 full power)
 }
}

    struct DC_motor motorL, motorR; //declare two DC_motor structures
    
void stop(struct DC_motor *m_L, struct DC_motor *m_R) {
    motorL.direction = 1;
    motorL.power = 0;
setMotorPWM(m_L);
    motorR.direction = 1;
    motorR.power = 0;
setMotorPWM(m_R);
    __delay_ms(200);
    motorL.direction = 1;
    motorL.power = 0;
setMotorPWM(m_L);
    motorR.direction = 1;
    motorR.power = 0;
setMotorPWM(m_R);
}
// turn robot base left on the spot
void turnLeft(struct DC_motor *m_L, struct DC_motor *m_R)
{
    motorL.direction=0;
    motorL.power=50;
    setMotorPWM(m_L);
    motorR.direction=1;
    motorR.power=60;
    setMotorPWM(m_R);
}
// turn robot base left on the spot
void turnRight(struct DC_motor *m_L, struct DC_motor *m_R)
{
    motorL.direction=1;
    motorL.power=60;
    setMotorPWM(m_L);
    motorR.direction=0;
    motorR.power=50;
    setMotorPWM(m_R);
}
// turn robot base right on the spot
void forward(struct DC_motor *m_L, struct DC_motor *m_R)
{
motorL.direction=1;
    motorL.power=73; // difference of %4 as the right motor runs faster than the left
    setMotorPWM(m_L);
    motorR.direction=1;
    motorR.power=71;
    setMotorPWM(m_R);
}
// both motors forward and up to full power
void back(struct DC_motor *m_L, struct DC_motor *m_R)
{
motorL.direction=0;
    motorL.power=73;
    setMotorPWM(m_L);
    motorR.direction=0;
    motorR.power=74;
    setMotorPWM(m_R);
}


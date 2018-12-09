/* 
 * File:   MotorControl.h
 * Author: rh2415
 *
 * Created on 22 November 2018, 16:00
 */

#ifndef MOTORCONTROL_H
#define	MOTORCONTROL_H
#define _XTAL_FREQ 8000000
#include <xc.h>
void initPWM(void);
void delay_s(char seconds);
struct DC_motor {
 char power; //motor power, out of 100
 char direction; //motor direction, forward(1), reverse(0)
 unsigned char *dutyLowByte; //PWM duty low byte address
 unsigned char *dutyHighByte; //PWM duty high byte address
 char dir_pin; // pin that controls direction on PORTB
 int PWMperiod; //base period of PWM cycle
};
void setMotorPWM(struct DC_motor *m);
void setMotorFullSpeed(struct DC_motor *m);
void stop(struct DC_motor *m_L, struct DC_motor *m_R);
void turnLeft(struct DC_motor *m_L, struct DC_motor *m_R);
void turnRight(struct DC_motor *m_L, struct DC_motor *m_R);
void forward(struct DC_motor *m_L, struct DC_motor *m_R);
void back(struct DC_motor *m_L, struct DC_motor *m_R);
#endif	/* MOTORCONTROL_H */


#include<string.h>
#include <pic18f4331.h>
#include <xc.h>
#include <stdio.h>

#define LCDP8 LATCbits.LATC0
#define _XTAL_FREQ 8000000 //i.e. for an 8MHz clock frequency
#pragma config OSC = IRCIO
// don?t forget to put function prototypes at the start of your program
// precise delays can be produced using the __delay_us() and __delay_ms()
// macro functions. For these to work you must write the line
// near the top of your main.c file so the functions can calculate how
// long to make the delay for different clock frequencies.

#define LCDP8 LATCbits.LATC0 // toggle pin
#define LCD_RS LATAbits.LATA6 // RS pin
#define LCD_B7 LATDbits.LATD1 // Pin 16 LCD
#define LCD_B6 LATDbits.LATD0 // Pin 15 LCD
#define LCD_B5 LATCbits.LATC2 // Pin 14 LCD
#define LCD_B4 LATCbits.LATC1// Pin 13 LCD


//function to toggle enable bit on then off
void E_TOG(void){
    LCDP8 = 1;
__delay_us(5); // 5us delay ? remember to define _XTAL_FREQ
    LCDP8 = 0;
}
//function to send four bits to the LCD
void LCDout(unsigned char number){
//set data pins using the four bits from number
 //toggle the enable bit to send data
LATD=((number&0b1100)>>2)|(LATD &0b11111100);
    LATC=(((number&0b0011)<<1)|(LATD &0b11111001))<<3;
//    if (number & 0b1000){LCD_B7=0b0001;} else{LCD_B7=0;}
//    if (number & 0b0100){LCD_B6=0b0001;} else{LCD_B6=0;}
//    if (number & 0b0010){LCD_B5=0b0001;} else{LCD_B5=0;}
//    if (number & 0b0001){LCD_B4=0b0001;} else{LCD_B4=0;}
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
    TRISA= 0;

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


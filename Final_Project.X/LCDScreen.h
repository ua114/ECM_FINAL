
#ifndef LCDSCREEN_H
#define	LCDSCREEN_H

void E_TOG(void);
void LCDout(unsigned char number);
void SendLCD(unsigned char ByteType,char type);
void LCD_Init(void);
void SetLine (char line);
void LCD_String(char *string);

#endif	/* LCD_SCREEN_H *//


#ifndef LCD_INTERFACE_H_
#define LCD_INTERFACE_H_

#include "DIO_interface.h"
/******************private***************/
#define _8_BIT 1
#define _4_BIT 2
extern u8 lcd_cursorCell;
extern u8 lcd_cursorLine;






/******************* config*******************************************/
#define EN PINC2
#define RS PINC3

#define LCD_MODE  _4_BIT
#define MIN_LINE 1
#define MAX_LINE 2
#define MIN_CELL 1
#define MAX_CELL 16
#define SPECIFIC_DIGITS 4 //print number on LCD 

/********8 bit mode **************/
#define  LCD_PORT PC


/************4 bit mode *********/
#define D7 PINC7
#define D6 PINC6
#define D5 PINC5
#define D4 PINC4 
/********************************************************************/


void LCD_Init(void);
void LCD_WriteNumber(s32 num);
void LCD_WriteNumber_4digits(s32 num);
void LCD_WriteBinary(u8 num);
void LCD_WriteHex(u8 num);
void LCD_WriteChar(u8 ch);
void LCD_WriteString(u8* str);
Error_type LCD_SetCursor(u8 line,u8 cell); //line 1,2 cell 1:16
void LCD_Clear(void);
void LCD_SnakeString(u8* str);
void LCD_SetShape(u8* pattern,u8 location);
void LCD_ClearCells(u8 line,u8 cell,u8 num);
extern u8 shape[8];


#endif /* LCD_INTERFACE_H_ */
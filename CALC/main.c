#define F_CPU 8000000UL
/*#include <util/delay.h>*/
#include "LCD_interface.h"
#include "KEYPAD.h"
#include "StdTypes.h"
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"
#include "queue.h"
#include "FreeRTOSConfig.h"




typedef struct{
	u8 arg1;
	u8 operation;
	u8 arg2;
	}equation_t;
	
typedef struct{
	u8 message[20];
    }display_t;	
	
typedef struct{      //inside calc task
	equation_t equa;
	s8 res;
    }result_t;
	
typedef struct{    //inside keypad task
	equation_t equ;
	u8 flag_complete;
	u8 flag_done;
	}buffer_t; 

typedef struct{
	u8 hours;
	u8 minutes;
	u8 seconds;
    }time_t;

	

TaskHandle_t keypad_task_handle=NULL, calculate_task_handle=NULL,display_task_handle=NULL;
TimerHandle_t  clock_time_handle=NULL,check_time_handle=NULL;
QueueHandle_t keypad2calc_queue=NULL,keypad2display_queue=NULL,calc2display_queue=NULL,clock2display_queue=NULL;
SemaphoreHandle_t clrLcd_sem=NULL,clrEquation_sem=NULL; 

 


void KeypadTask_Func(void)
{
	u8 Local_key=NO_KEY;
	u8 Local_index={0};
	buffer_t Local_equation_buffer;
	BaseType_t Local_ObjectState = pdFALSE;
	
	while(1)
	{
		Local_key=KEYPAD_GetKey();
		/* analyze the keypad input and the equation index */
		if(Local_key != NO_KEY)
		{
			/* the 1st argument of the equation*/
			if(Local_index==0 && (Local_key>='0' && Local_key<='9'))
			{
				Local_equation_buffer.equ.arg1 =Local_key;
				Local_equation_buffer.equ.operation=NO_KEY;
				Local_equation_buffer.equ.arg2=NO_KEY;
				Local_equation_buffer.flag_complete=0;
				Local_index=(Local_index+1)%4;
				/* start the check timer */
				xTimerStop(check_time_handle,0);
				xTimerStart(check_time_handle,0);
				/* send equation to lcd */
				xQueueSend(keypad2display_queue,&Local_equation_buffer,0);
			}
			else if(Local_index==1 && (Local_key=='-' || Local_key=='+' || Local_key=='/' || Local_key=='*' ))
			{
				Local_equation_buffer.equ.operation=Local_key;
				Local_equation_buffer.equ.arg2=NO_KEY;
				Local_equation_buffer.flag_complete=0;
				Local_index=(Local_index+1)%4;
				/* start the check timer */
				xTimerStop(check_time_handle,0);
				xTimerStart(check_time_handle,0);
				/* send equation to lcd */
				xQueueSend(keypad2display_queue,&Local_equation_buffer,0);
			}
			else if(Local_index==2 && (Local_key>='0' && Local_key<='9'))
			{
				Local_equation_buffer.equ.arg2=Local_key;
				Local_equation_buffer.flag_complete=0;
				Local_index=(Local_index+1)%4;
				/* start the check timer */
				xTimerStop(check_time_handle,0);
				xTimerStart(check_time_handle,0);
				/* send equation to lcd */
				xQueueSend(keypad2display_queue,&Local_equation_buffer,0);
			}
			else if(Local_index==3 && (Local_key=='='))
			{
				Local_equation_buffer.flag_complete=1;
				Local_index=(Local_index+1)%4;
				/* stop the check timer */
				xTimerStop(check_time_handle,0);
				/* send equation to lcd */
				xQueueSend(keypad2display_queue,&Local_equation_buffer,0);
			}
			else {/* do nothing */}
			/* send equation to calc if all parameters are existed */
			if(Local_equation_buffer.flag_complete==1)
			{
				xQueueSend(keypad2calc_queue,&Local_equation_buffer.equ,0);
				/* empty the full equation */
				Local_equation_buffer.equ.arg1=NO_KEY;
				Local_equation_buffer.equ.operation=NO_KEY;
				Local_equation_buffer.equ.arg2=NO_KEY;
				Local_equation_buffer.flag_complete=0;
			}
			else {/* do nothing */}
			/* check time: if 5 sec is out */
			Local_ObjectState=xSemaphoreTake(clrEquation_sem,0);
			if(Local_ObjectState== pdPASS)
			{
				/* empty the full equation */
				Local_equation_buffer.equ.arg1=NO_KEY;
				Local_equation_buffer.equ.operation=NO_KEY;
				Local_equation_buffer.equ.arg2=NO_KEY;
				Local_equation_buffer.flag_complete=0;
				/* initiate the equation index */
				Local_index=0;
			}
			else {/* do nothing */}
		}
		else {/* do nothing */}
	}
}

void CalcTask_Func(void)
{
	result_t local_answer={0};
	BaseType_t Local_objectState=pdFALSE;
	while(1)
	{
		/* try to take the equation from keypad */
		Local_objectState=xQueueReceive(keypad2calc_queue,&local_answer.equa,0);
		if(Local_objectState==pdPASS)
		{
			switch(local_answer.equa.operation)
			{
				case '+':
				local_answer.res=(local_answer.equa.arg1-'0')+(local_answer.equa.arg2-'0');
				break;
				case '-':
				local_answer.res=(local_answer.equa.arg1-'0')-(local_answer.equa.arg2-'0');
				break;
				case '*':
				local_answer.res=(local_answer.equa.arg1-'0')*(local_answer.equa.arg2-'0');
				break;
				case '/':
				if(local_answer.equa.arg2==0)
				{
					local_answer.res=0; /* may it handle in future */
				}
				else
				{
					local_answer.res=(local_answer.equa.arg1-'0')/(local_answer.equa.arg2-'0');
				}
				break;
				default:
				/* do nothing */
				break;
			}
			/* send the equation with the answer to lcd */
			xQueueSend(calc2display_queue,&local_answer,0);
		}
	}
}
void DisplayTask_Func(void)
{
	buffer_t Local_equation_buffer={0};
	result_t local_answer={0};
	time_t local_time={0};
	BaseType_t Local_objectState=pdFALSE;
	while(1)
	{
		/* check if time is out */
		Local_objectState=xSemaphoreTake(clrLcd_sem,0);
		if(Local_objectState==pdPASS)
		{
			LCD_SetCursor(1,1);
			LCD_WriteString("            "); // to remove any previous equation 
		}
		else{/* do nothing */}
		/* try to take the equation from keypad */
		Local_objectState=xQueueReceive(keypad2display_queue,&Local_equation_buffer,0);
		if(Local_objectState==pdPASS)
		{
			LCD_SetCursor(1,1);
			if((Local_equation_buffer.equ.arg1)!= NO_KEY)
			{
				LCD_WriteChar(Local_equation_buffer.equ.arg1);
			}
			else 
			{
				LCD_WriteChar(' '); 
			}
			if((Local_equation_buffer.equ.operation)!= NO_KEY)
			{
				LCD_WriteChar(Local_equation_buffer.equ.operation);
			}
			else
			{
				LCD_WriteChar(' ');
			}
			if((Local_equation_buffer.equ.arg2)!= NO_KEY)
			{
				LCD_WriteChar(Local_equation_buffer.equ.arg2);
			}
			else
			{
				LCD_WriteChar(' ');
			}	
			if(Local_equation_buffer.flag_complete==1)
			{
				LCD_WriteChar('=');
			}
			else
			{
				LCD_WriteString("     ");  // to remove any previous result
			}
		}
		else{/* do nothing */}
		/* try to take the full equation with answer from calc */
		Local_objectState=xQueueReceive(calc2display_queue,&local_answer,0);
		if(Local_objectState==pdPASS)
		{
			LCD_SetCursor(1,1);
			LCD_WriteChar(local_answer.equa.arg1);
			LCD_WriteChar(local_answer.equa.operation);
			LCD_WriteChar(local_answer.equa.arg2);
			LCD_WriteChar('=');
			LCD_WriteNumber(local_answer.res);
		}
		else{/* do nothing */}
		/* try to take time queue */
		Local_objectState=xQueueReceive(calc2display_queue,&local_time,0);
		if(Local_objectState==pdPASS)
		{
			/* change time on lcd */
			LCD_SetCursor(2,1);
			LCD_WriteNumber(local_time.hours);
			LCD_WriteChar(':');
			LCD_WriteNumber(local_time.minutes);
			LCD_WriteChar(':');
			LCD_WriteNumber(local_time.seconds);
			LCD_WriteString("    "); /* to remove any previous time */
		}
	}
}
void ClockTime_Func(void)
{
	static time_t local_time={0};
	BaseType_t local_objectState=pdFALSE;
	/* try to take time queue */
	local_time.seconds++;
	if(local_time.seconds==60)
	{
		local_time.seconds=0;
		local_time.minutes++;
	}
	else{/* do nothing*/}
	if(local_time.minutes==60)
	{
		local_time.minutes=0;
		local_time.hours++;
	}
	else{/* do nothing*/}
	if(local_time.hours==24)
	{
		local_time.hours=0;
	}
	local_objectState=xQueueSend(clock2display_queue,&local_time,0);	
}
void CheckTime_Func(void)
{
	/* give time out semaphore to clear lcd in lcd task */
	xSemaphoreGive(	clrLcd_sem);
	/* give time out semaphore to clear the equation in keypad task*/
	xSemaphoreGive(clrEquation_sem);
	xTimerStop(check_time_handle,0); /* start in keypad task*/
}

void lcd_time(void)
{
	
	
}
int main ()
{
	/* init hal */
	LCD_Init();
	KEYPAD_Init();
	/* create semaphores */
	clrEquation_sem=xSemaphoreCreateBinary();
	clrLcd_sem=xSemaphoreCreateBinary();
	/* create queues */
	keypad2calc_queue=xQueueCreate((u8)1,sizeof(equation_t));
	keypad2display_queue=xQueueCreate((u8)1,sizeof(buffer_t));
	calc2display_queue=xQueueCreate((u8)1,sizeof(result_t));
	clock2display_queue=xQueueCreate((u8)1,sizeof(time_t));
	/* create timers */
	clock_time_handle=xTimerCreate(NULL,pdMS_TO_TICKS(1000),pdTRUE,NULL,ClockTime_Func);
	check_time_handle=xTimerCreate(NULL,pdMS_TO_TICKS(50000),pdTRUE,NULL,CheckTime_Func); 
	/* create tasks */
	keypad_task_handle=xTaskCreate(&KeypadTask_Func,NULL,250,NULL,3,NULL);
	display_task_handle=xTaskCreate(&DisplayTask_Func,NULL,250,NULL,1,NULL);
	calculate_task_handle=xTaskCreate(&CalcTask_Func,NULL,250,NULL,2,NULL);
	/* start timers */
	xTimerStart(clock_time_handle,0);
	xTimerStart(check_time_handle,0); 
	/* start scheduler */
	vTaskStartScheduler();
	return 0;
}

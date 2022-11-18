/*
 * main.c
 *
 *  Created on: Nov 5, 2022
 *  Author: omar hamdii
 */


#include "lcd.h"
#include "uart.h"
#include "keypad.h"
#include "std_types.h"
#include <avr/io.h> /* To use the SREG register */
#include <util/delay.h> /* For the delay functions */
#include <avr/interrupt.h>

#include "Timer1/Timer1.h"

/*******************************Timer1_Decleration*****************************/
/*choosing the mode of timer 1
 * intial value t0 count=0
 * compare mode value=0
 * prescale 64
 * Normal mode
 */
Timer1_ConfigType confige_timer = {0 , 977 , prescaler_1024 , Compare_Mode} ;

/*variable that count on every on second*/
uint8 tick=0;

void Timer1 (void)
{
	/*increment 1 every one second*/
	tick++;
}

/**********************************UART_Decleration*************************/
/*choosing mode of the uart
 * 9600 buate rate
 * 8_bite transfering data
 * 1_bit stop bite
 * disable parity bite
 */
UART_ConfigType confige ={9600,bit_8,bit_1,Disable};

/*intialize variable that store byte send from mc2 in it*/
uint8 recieve_byte =0;
void UART_recieveByte (void)
{
	/*
	 * Read the received data from the Rx buffer (UDR)
	 * The RXC flag will be cleared after read the data
	 */
	recieve_byte = UDR ;
}

/************************Global_variabe***************************************/
/*variable that we store value of button pressed*/
uint8 key_num ;

/*indicate that paswword is end*/
uint8 Endofpass = '#';

/*indicate that paswword is wrong*/
uint8 wrongpass = '&' ;

/*indicate that user enter password not matched with EEPROM*/
uint8 wrongpass2= '*';

/*indicate that password is right*/
uint8 Rightpass = '$' ;

/*stage that indicate user enter a first passwrod to store*/
uint8 passstage = 1;

/*stage that user user enter two matched password and has option to
 *  open door
 *  change password
 */
uint8 secondstage=0;

/*variable indicate how many tries user try to enter a password*/
uint8 counter=0;

/*user choose to open the door*/
uint8 Enter_password=1;

/*showing door state
 * opening
 * closing
 */
uint8 UART_Byte=0;

/*indicate user want to
 * change password
 * open door
 */
uint8 state = 0;


/*function checking if user enter right password or not
 * to change password
 * open door
 */
uint8 checking (void)
{
	/*indicate user want to open door*/
	if (state==1)
	{
		UART_sendByte('+');
	}
	/*indicate user want to change password*/
	else if (state==2)
	{
		UART_sendByte('-');
	}

	LCD_clearScreen();
	LCD_displayStringRowColumn(0, 0, " Plz enter pass!");
	Enter_password = 1;
	UART_Byte=0;
	while (Enter_password)
	{

		key_num=KEYPAD_getPressedKey();

		if (key_num== '=')
		{
			UART_sendByte(Endofpass);
			Enter_password=0;
			UART_Byte=1;
		}
		else
		{
			LCD_displayCharacter('*');
			UART_sendByte(key_num);
		}
		_delay_ms(500); /* Press time */
	}


	while (UART_Byte)
	{
		if (recieve_byte == Rightpass)
		{
			if (state==1)
			{
				LCD_clearScreen();
				/*setting call_back function to call Timer1*/
				Timer1_setCallBack(Timer1);
				/*Intialize Timer1 Compare mode*/
				Timer1_init(&confige_timer);
				LCD_displayStringRowColumn(0, 0, " Door is opening");
				/*waiting 15 seconds*/
				while (tick<=15);
				{
					LCD_clearScreen();
					LCD_displayStringRowColumn(0, 0, " Door is opened");
				}
				/*waiting 18 seconds*/
				while (tick<=18);
				{
					LCD_displayStringRowColumn(0, 0, " Door is closing");
				}
				/*waiting 33 seconds*/
				while (tick<=33);
				{
					LCD_clearScreen();
					Timer1_deInit();
					tick=0;
					return 2;
				}
			}
			else if (state==2)
			{
				passstage=1;
				secondstage=0;
				LCD_clearScreen();
				break;
			}
		}


		else if (recieve_byte==wrongpass2)
		{
			return 1;
		}
	}
}


void main ()
{

	DDRB |= (1<<6);
	PORTB &= ~(1<<6);
	DDRB |= (1<<7);
	PORTB &= ~(1<<7);

	/* Initialize the LCD Driver */
	LCD_init() ;

	/*Intializing of Uart*/
	UART_init(&confige);

	/* Set the Call back function pointer in the ICU driver */
	UART_setCallBack(UART_recieveByte);

	/* Enable Global Interrupt I-Bit */
	SREG |= (1<<7);


	while (1)
	{

		if (counter==0)
		{
			LCD_displayStringRowColumn(0, 0, "Plz enter pass!");
			LCD_moveCursor(1, 0);
		}


		while (passstage)
		{

			key_num = KEYPAD_getPressedKey();


			if (key_num== '=')
			{
				if (counter == 0)
				{
					LCD_clearScreen();
					LCD_displayStringRowColumn(0, 0, "Plz re_enter pass!");
				}
				UART_sendByte(Endofpass);
				counter++;
			}
			else
			{
				LCD_displayCharacter('*');
				UART_sendByte(key_num);
			}

			_delay_ms(500); /* Press time */

			/*if user enter two matched password*/
			if ((recieve_byte == Rightpass) && (counter==2))
			{
				LCD_clearScreen();
				passstage=0;
				secondstage=1;
				counter=0;
				break ;

			}

			/*if user enter not matched password*/
			if ((recieve_byte==wrongpass)  &&  (counter==2))
			{
				LCD_clearScreen();
				LCD_displayStringRowColumn(0, 0, "wrong pass! Plz enter pass!");
				LCD_moveCursor(1, 0);
				counter=0;
				UART_sendByte(wrongpass);

			}

		}/*End while of passstage*/




		/*indicate second stage user want to
		 * open door
		 * change password
		 */
		while (secondstage)

		{
			uint8 data;
			LCD_clearScreen();
			LCD_displayStringRowColumn(0, 0, "+: open the door");
			LCD_displayStringRowColumn(1, 0, "-: change  pass!");
			key_num=KEYPAD_getPressedKey();

			/*First if indicate that user want to open_door*/
			if  (key_num == '+')
			{
				state=1;
				data = checking();
				if (data==1)
				{
					data=checking();
					if (data==1)
					{
						data=checking();

						if (data==1)
						{
							LCD_clearScreen();
							LCD_displayStringRowColumn(0, 0, " Error!!!");
							/*setting call_back function to call Timer1*/
							Timer1_setCallBack(Timer1);
							/*Intialize Timer1 Compare mode*/
							Timer1_init(&confige_timer);
							while (tick<=100);
						}
					}

				}

			}/*End if of user want to open_door*/


			/*Indicate that user want to change the password*/
			else if (key_num == '-')
			{
				state=2;
				data = checking();
				if (data==1)
				{
					data=checking();
					if (data==1)
					{
						data=checking();
						PORTB |= (1<<6);

						if (data==1)
						{
							LCD_clearScreen();
							LCD_displayStringRowColumn(0, 0, " Error!!!");
							/*setting call_back function to call Timer1*/
							Timer1_setCallBack(Timer1);
							/*Intialize Timer1 Compare mode*/
							Timer1_init(&confige_timer);
							while (tick<=100);
						}
					}

				}

			}/*End if change password*/


		}/*End while (secondstage)*/





	}/*End of while 1*/



}

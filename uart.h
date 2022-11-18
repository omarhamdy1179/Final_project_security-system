 /******************************************************************************
 *
 * Module: UART
 *
 * File Name: uart.h
 *
 * Description: Header file for the UART AVR driver
 *
 * Author: Mohamed Tarek
 *
 *******************************************************************************/

#ifndef UART_H_
#define UART_H_

#include "std_types.h"

typedef enum
{
	Asynchronous,synchronous
}Mode_select;

typedef enum
{
	Disable,Even_parity=2,Odd_parity=3
}UART_Parity;

typedef enum
{
bit_1,bit_2

}UART_StopBit;

typedef enum
{
	bit_5, bit_6 ,bit_7 ,bit_8
}UART_BitData;


typedef struct
{
uint16 UART_BaudRate ;
UART_BitData data_bit ;
UART_StopBit bit_stop ;
UART_Parity parity ;
}UART_ConfigType;


/*******************************************************************************
 *                      Functions Prototypes                                   *
 *******************************************************************************/

/*
 * Description :
 * Functional responsible for Initialize the UART device by:
 * 1. Setup the Frame format like number of data bits, parity bit type and number of stop bits.
 * 2. Enable the UART.
 * 3. Setup the UART baud rate.
 */
void UART_init(const UART_ConfigType * Config_Ptr);

/*
 * Description :
 * Functional responsible for send byte to another UART device.
 */
void UART_sendByte(const uint8 data);


void UART_setCallBack(void(*a_ptr)(void));

/*
 * Description :
 * Send the required string through UART to the other UART device.
 */
void UART_sendString(const uint8 *Str);

/*
 * Description :
 * Receive the required string until the '#' symbol through UART from the other UART device.
 */
void UART_receiveString(uint8 *Str); // Receive until #

#endif /* UART_H_ */

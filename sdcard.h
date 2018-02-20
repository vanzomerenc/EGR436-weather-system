/*
 * sdcard.h
 *
 *  Created on: Feb 19, 2018
 *      Author: Samanth
 */

#ifndef SDCARD_H_
#define SDCARD_H_

#include <stdio.h>
#include <driverlib.h>
#include <string.h>
#include <ctype.h>
#include "rtc.h"

enum TimeSetState { NOT_SETTING, SETTING_MONTH, SETTING_DAY, SETTING_YEAR, SETTING_HOUR, SETTING_MINUTE, SETTING_SECOND };
enum TimeSetState timeSetState = NOT_SETTING;
char UARTBuffer[256] = {'\0'};
char newInput[256] = {'\0'};
int newInputReceived = 0;

/* UART Configuration Parameter. These are the configuration parameters to
 * make the eUSCI A UART module to operate with a 115200 baud rate. These
 * values were calculated using the online calculator that TI provides
 * at:
 * http://processors.wiki.ti.com/index.php/
 *               USCI_UART_Baud_Rate_Gen_Mode_Selection
 */
const eUSCI_UART_Config uartConfig =
{
        EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
        208,                                      // BRDIV = 26
        0,                                       // UCxBRF = 0
        0,                                       // UCxBRS = 0
        EUSCI_A_UART_NO_PARITY,                  // No Parity
        EUSCI_A_UART_LSB_FIRST,                  // MSB First
        EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
        EUSCI_A_UART_MODE,                       // UART mode
        EUSCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION  // Low Frequency Mode
};

/*
 * USCIA0 interrupt handler.
 */
void EusciA0_ISR(void)
{
    int receiveByte = UCA0RXBUF;

    // Will use this as "enter" for the user.
    if(receiveByte == 13) // This is sent from a carriage return
    {
        newInputReceived = 1;
        strcpy(newInput, UARTBuffer);
        UARTBuffer[0] = '\0';
        //__no_operation();
    }
    else
    {
        if(isdigit(receiveByte))
        {
            char tempS[2];
            tempS[0] = receiveByte;
            tempS[1] = '\0';
            strcat(UARTBuffer, tempS);
        }
    }
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);

    /* Echo back. */
    EUSCI_A_UART_transmitData(EUSCI_A0_BASE, receiveByte);

    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN0);
}

// For tera term communication using the UART backchannel
void initUART()
{
    /* Selecting P1.2 and P1.3 in UART mode. */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
        GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);

    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT | EUSCI_A_UART_BREAKCHAR_INTERRUPT);
    Interrupt_enableInterrupt(INT_EUSCIA0);
}

void processUART(struct rtc_time* time)
{
    if(newInputReceived)
    {
        switch(timeSetState)
        {
            case NOT_SETTING:
                printf("\nEnter the month\n");
                break;
            case SETTING_MONTH:
                sscanf(newInput, "%d", &time->month);
                printf("\nEnter the day\n");
                break;
            case SETTING_DAY:
                sscanf(newInput, "%d", &time->date);
                printf("\nEnter the year\n");
                break;
            case SETTING_YEAR:
                sscanf(newInput, "%d", &time->year);
                printf("\nEnter the hour\n");
                break;
            case SETTING_HOUR:
                sscanf(newInput, "%d", &time->hour);
                printf("\nEnter the minute\n");
                break;
            case SETTING_MINUTE:
                sscanf(newInput, "%d", &time->min);
                printf("\nEnter the second\n");
                break;
            case SETTING_SECOND:
                sscanf(newInput, "%d", &time->sec);
                printf("\nTime set.\n");
                break;

        }
        if(timeSetState == SETTING_SECOND)
        {
            rtc_settime(time);
            timeSetState = NOT_SETTING;
        }
        else
        {
            ++timeSetState;
        }
        newInputReceived = 0;
    }
}

int fputc(int _c, register FILE *_fp)
{
    while(!(UCA0IFG&UCTXIFG));
    UCA0TXBUF = (unsigned char)_c;
    return ((unsigned char)_c);
}

int fputs(const char *_ptr, register FILE *_fp)
{
    unsigned int i, len;
    len = strlen(_ptr);

    for(i = 0; i < len; i++)
    {
        while(!(UCA0IFG&UCTXIFG));
        UCA0TXBUF = (unsigned char) _ptr[i];
    }

    return len;
}


#endif /* SDCARD_H_ */

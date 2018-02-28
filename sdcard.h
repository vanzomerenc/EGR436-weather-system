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
        // Set flag indicating new input available in buffer
        newInputReceived = 1;
        // Copy the buffer to the newInput global string so we don't lose it
        strcpy(newInput, UARTBuffer);
        // Set the first byte of buffer to a null char so we can start a new string
        UARTBuffer[0] = '\0';
        //__no_operation();
    }
    else
    {
        // Check for valid input
        if(isdigit(receiveByte))
        {
            // Convert character to null-terminating string
            char tempS[2];
            tempS[0] = receiveByte;
            tempS[1] = '\0';
            // Concat the char string with the bytes already stored in buffer
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
    // This tracks a state machine that keeps track of where the user
    // is in the process of setting the date/time
    if(newInputReceived)
    {
        switch(timeSetState)
        {
        // first prompt
            case NOT_SETTING:
                printf("\n\rEnter the month\n\r");
                break;
            case SETTING_MONTH: // Grab int from serial in, stored as month
                sscanf(newInput, "%d", &time->month);
                printf("\n\rEnter the day\n\r");
                break;
            case SETTING_DAY: // Store int from serial in as day of month
                sscanf(newInput, "%d", &time->date);
                printf("\n\rEnter the year\n\r");
                break;
            case SETTING_YEAR: // Store int from serial in as year
                sscanf(newInput, "%d", &time->year);
                printf("\n\rEnter the hour\n\r");
                break;
            case SETTING_HOUR:  // hour
                sscanf(newInput, "%d", &time->hour);
                printf("\n\rEnter the minute\n\r");
                break;
            case SETTING_MINUTE: // minutes
                sscanf(newInput, "%d", &time->min);
                printf("\n\rEnter the second\n\r");
                break;
            case SETTING_SECOND: // seconds
                sscanf(newInput, "%d", &time->sec);
                printf("\n\rTime set.\n\r");
                break;

        }
        if(timeSetState == SETTING_SECOND)
        {
            // If we got to the last stage, go ahead and send the struct on
            rtc_settime(time);
            // Print formatted results to terminal
            printf("%d-%d-%d %d:%d:%d\n\r", time->month
                   , time->date, time->year, time->hour
                   , time->min, time->sec);
            // Set the state machine back to the start
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

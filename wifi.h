/**
 * @author Samantha Teskey
 * @date 3-20-2018
 * Prelab 9
 */
#ifndef WIFI_H_
#define WIFI_H_

/* MSP432 */
#include "driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

/**
 * UART configuration parameters to make eUSCI A UART module operate with
 * a 115200 baud rate.
 */
const eUSCI_UART_Config uartConfigWifi =
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK, // SMCLK at 12000000
    6,                              // BRDIV = INT((12000000/115200=104.167)/16)
    8,                              // UCxBRF INT([(104.167/16)-INT(104.167/16)]x16))=(6.51-6)x16=8.16)
    0x11,                           // UCxBRS = 0x11 (frac part of N from p. 721)
    EUSCI_A_UART_NO_PARITY,
    EUSCI_A_UART_LSB_FIRST,
    EUSCI_A_UART_ONE_STOP_BIT,
    EUSCI_A_UART_MODE,
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION
};

/* Initialization for WiFi module */
void wifiInit()
{
    /* Selecting P1.2 and P1.3 in UART mode. */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
        GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Selecting P3.2 and P3.3 in UART mode. */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
        GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfigWifi);
    MAP_UART_initModule(EUSCI_A2_BASE, &uartConfigWifi);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);
    MAP_UART_enableModule(EUSCI_A2_BASE);

    UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT | EUSCI_A_UART_BREAKCHAR_INTERRUPT);
    UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT | EUSCI_A_UART_BREAKCHAR_INTERRUPT);

    Interrupt_enableInterrupt(INT_EUSCIA0);
    Interrupt_enableInterrupt(INT_EUSCIA2);
}

/* Globals for WiFi module */
uint8_t U0RXData; // A0 UART (terminal)
uint8_t U2RXData; // A2 UART (ESP8266)

/* EUSCI A0 UART ISR - gets data from PC terminal emulator, sends to ESP8266 connected to UART2 */
void euscia0_isr(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

    MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT)
    {
        U0RXData = UART_receiveData(EUSCI_A0_BASE);
        if(U0RXData == 13)
        {
            MAP_UART_transmitData(EUSCI_A2_BASE, '\r\n');
        }
        else
        {
            MAP_UART_transmitData(EUSCI_A2_BASE, U0RXData); // send byte out UART2 port
        }
    }
}

/* EUSCI A2 UART ISR - gets data from ESP8266 on UART2, sends to terminal emulator on UART0 */
void euscia2_isr(void)
{
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);
    MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT)
    {
        U2RXData = MAP_UART_receiveData(EUSCI_A2_BASE);
        MAP_UART_transmitData(EUSCI_A0_BASE, U2RXData); // send byte out UART0 port
    }
}
#endif // WIFI_H_

#ifndef RECEIVER_H_
#define RECEIVER_H_

/* DriverLib Includes */
#include "driverlib.h"

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>

#include <drv/nrf24/msprf24.h>
#include <drv/rtc.h>
#include <sensors/atmospheric.h>
#include "comm.h"

static struct comm_message received_message;

struct rtc_time received_time;
struct sensor_atmospheric_result received_atmospheric_reading;
float received_light;

void receiverInit()
{
    spi_init();

    rf_crc = RF24_EN_CRC | RF24_CRCO; // CRC enabled, 16-bit
    rf_addr_width      = 5;
    rf_speed_power     = RF24_SPEED_1MBPS | RF24_POWER_0DBM;
    rf_channel = 42;

    msprf24_init();

    char addr[5];
    msprf24_open_pipe(0, 1);  // Open pipe#0 with Enhanced ShockBurst

    // Set our RX address
    //addr[0] = 0xDE; addr[1] = 0xAD; addr[2] = 0xBE; addr[3] = 0xEF; addr[4] = 0x00;
    addr[0] = 'B'; addr[1] = 'E'; addr[2] = 'A'; addr[3] = 'N'; addr[4] = 'S';
    w_rx_addr(0, addr);

    // Receive mode
    if (!(RF24_QUEUE_RXEMPTY & msprf24_queue_state())) {
        flush_rx();
    }

    GPIO_interruptEdgeSelect (GPIO_PORT_P2, GPIO_PIN5, GPIO_HIGH_TO_LOW_TRANSITION );
    MAP_GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN5);
    MAP_Interrupt_enableInterrupt(INT_PORT2);
    Interrupt_enableMaster();
    msprf24_activate_rx();
    msprf24_irq_clear(RF24_IRQ_RX);

    int isAlive = msprf24_is_alive();
}

void receiverRoutine()
{
    if (rf_irq & RF24_IRQ_FLAGGED) {
        msprf24_get_irq_reason();
        delay_ms(10);
        if (rf_irq & RF24_IRQ_RX) {
            r_rx_payload(32, received_message.data);
            msprf24_irq_clear(RF24_IRQ_RX);
            switch(received_message.data[0])
            {
            case COMM_MESSAGE_RTC_TIME:
                comm_decode_rtc_time(&received_message, &received_time);
                break;
            case COMM_MESSAGE_SENSORS:
                comm_decode_sensor_readings(&received_message, &received_atmospheric_reading, &received_light);
                break;
            default:
                break;
            }

            P2OUT ^= BIT0;
        }
        msprf24_irq_clear(rf_irq);
    }
}

#endif // RECEIVER_H_

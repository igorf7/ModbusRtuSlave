#include "main.h"
#include "leds.h"
#include "uart.h"
#include "crc16.h"
#include "mbrtuslave.h"
#include "scheduler.h"

uint16_t MyAddress = MY_DEVICE_ID;
UartEvents_t UartEvents;
uint8_t BusyMessage[5];
AppBuffer_t AppBuffer;
uint8_t app_buffer[APP_BUFF_SIZE];

/*!
 \brief Main function of the application
 */
int main(void)
{
    AppBuffer.isBusy = false;
    AppBuffer.rtu_frame = (RtuFrame_t*)app_buffer;
    
    /* Initialize UART */
    UartEvents.uartTxReady = NULL;
    UartEvents.uartTxComplete = NULL;
    UartEvents.uartRxByte = NULL;
    UartEvents.uartError = onUartError;
    UartEvents.uartRxBuff = onUartRxBuff;
    
	initUart(115200, &UartEvents);
    
    /* Initialize LEDs to indicate errors */
    ledInit(LED1); // UART frame error
    ledInit(LED2); // UART noise error
    ledInit(LED3); // UART overrun error
    ledInit(LED4); // MCU Hard fault
    
#ifdef DEBUG_CONFIG
    initModbusRegisters();
#endif
    
    /* Initialize the task queue */
    initTaskQueue(&backgroundTask);

    /* Initialize Watchdog timer */
    fwdgt_config(0x0FFF, FWDGT_PSC_DIV64);
    fwdgt_enable();
    
    __enable_irq();
    
    /* Mainloop */
    while (1)
    {
        runTaskSheduler(); // task management
    }
}

/*!
 \brief Background task callback
 */
void backgroundTask(void)
{
    fwdgt_counter_reload(); // reload watchdog timer
    __nop();
}

/*!
 \brief On Uart Errors callback
 \param [IN] err - UART error code
 */
void onUartError(uint16_t err)
{
    int i;
    
    for (i = 0; i < USART_ERRORS_NUM; i++) {
        if (err & (0x1 << i)) {
            turnLedOn((led_enum_t)i);
        }
        else {
            turnLedOff((led_enum_t)i);
        }
    }
}

/*!
 \brief On Uart Packet Rceived callback
 \param [IN] *data - pointer to uart driver data buffer
 \param [IN] len - size of received data in bytes
 */
void onUartRxBuff(uint8_t *uart_buff, uint32_t len)
{
    register uint16_t checkSum;
    
    if (uart_buff[0] != MyAddress) {
        return; // Not my address, do nothing...
    }
    // Checksum calculation and verification
    checkSum = (uart_buff[len-1] << 8) | uart_buff[len-2];
    if (checkSum != calcBuffCrc16(uart_buff, len-2)) {
        return; // Checksum error! Do nothing...
    }
    if (!AppBuffer.isBusy) {
        AppBuffer.isBusy = true; //
        // Copy data from uart driver buffer to application buffer
        memcpy(AppBuffer.rtu_frame, uart_buff, len);
        // Add a data parsing task to the processing queue
        putEvent(parseReceivedPacket, &AppBuffer);
    }
    else { // Device is busy, send "busy" message
        BusyMessage[0] = MyAddress;
        BusyMessage[1] = (uart_buff[1] | 0x80);
        BusyMessage[2] = SLAVE_DEVICE_BUSY;
        checkSum = calcBuffCrc16(BusyMessage, 3);
        BusyMessage[3] = (uint8_t)checkSum;
        BusyMessage[4] = (uint8_t)(checkSum >> 8);
        sendResponse((uint32_t)BusyMessage, (uint32_t)sizeof(BusyMessage));
    }
}
// eof

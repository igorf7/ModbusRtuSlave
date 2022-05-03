#ifndef __UART_H
#define __UART_H

#include "common.h"

#define sendResponse startDmaMemToPeriph

#define USART0_RDATA_ADDRESS      ((uint32_t)&USART_RDATA(USART0))
#define USART0_TDATA_ADDRESS      ((uint32_t)&USART_TDATA(USART0))

#define UART_BUFF_SIZE   256

#define USART_ERRORS_NUM    (uint8_t)3
/* UART error flags */
#define UART_NO_ERROR       (uint8_t)0x00
#define UART_FERR_FLAG      (uint8_t)0x01
#define UART_NERR_FLAG      (uint8_t)0x02
#define UART_ORERR_FLAG     (uint8_t)0x04

/*!
 \brief UART buffer
 */
 typedef struct {
     uint32_t len;
     uint8_t *data;
 }UartBuffer_t;

/*!
 \brief UART driver callback functions
 */
typedef struct {
    void(*uartError)(uint16_t err);
    void(*uartRxByte)(uint8_t data);
    void(*uartRxBuff)(uint8_t *data, uint32_t len);
    void(*uartTxReady)(void);
    void(*uartTxComplete)(void);
} UartEvents_t;


/*** UART DRIVER API ***/
void initUart(uint32_t br, UartEvents_t *events);
void startDmaMemToPeriph(uint32_t m_addr, uint32_t size);

#endif //__UART_H
// eof

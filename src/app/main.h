#ifndef __MAIN_H
#define __MAIN_H

#include "common.h"
#include "mbrtuslave.h"

#define MY_DEVICE_ID    (uint8_t)2
#define APP_BUFF_SIZE    256U

typedef struct {
    bool isBusy;
    RtuFrame_t *rtu_frame;
} AppBuffer_t;

/*** Application API ***/
void backgroundTask(void);
void onUartError(uint16_t err);
void onUartRxBuff(uint8_t *uart_buff, uint32_t len);
#endif //__MAIN_H
//eof

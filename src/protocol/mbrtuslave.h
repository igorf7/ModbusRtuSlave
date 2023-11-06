#ifndef __MBRTUSLAVE_H
#define __MBRTUSLAVE_H

#include "common.h"

#define HEADER_SIZE                 2
#define CHECKSUM_SIZE               2

#define MAX_PDU_SIZE                252 // bytes
#define NUMBER_OF_DOUTS             256 // bytes
#define NUMBER_OF_DINPS             256 // bytes
#define NUMBER_OF_HOLDING_REGS      256 // Words
#define NUMBER_OF_INPUT_REGS        256 // Words
#define HIGH                        0xFF
#define LOW                         0x00

/* Mobbus RTU Exception Codes */
typedef enum {
    NO_ERROR = 0,
    ILLEGAL_FUNCTION,
    ILLEGAL_DATA_ADDRESS,
    ILLEGAL_DATA_VALUE,
    SLAVE_DEVICE_FAILURE,
    ACKNOWLEDGE,
    SLAVE_DEVICE_BUSY,
    NEGATIVE_ACKNOWLEDGE,
    MEMORY_PARITY_ERROR
} Exception_t;

/* Mobbus RTU frame stucture */
#pragma pack(push, 1)
typedef struct {
    uint8_t id;
    uint8_t func;
    uint8_t pdu[];
} RtuFrame_t;
#pragma pack(pop)

/*** MODBUS RTU DRIVER API ***/
void initModbusRegisters(void);
void parseReceivedPacket(void *prm);
#endif // __MBRTUSLAVE_H
// eof

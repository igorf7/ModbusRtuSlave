#include "mbrtuslave.h"
#include "main.h"
#include "uart.h"
#include "crc16.h"

static uint16_t HoldingRegisters[NUMBER_OF_HOLDING_REGS];
static uint16_t InputRegisters[NUMBER_OF_INPUT_REGS];
static uint8_t DiscreteOutputs[NUMBER_OF_DOUTS];
static uint8_t DiscreteInputs[NUMBER_OF_DINPS];
static uint8_t ExceptionCode = 0;

/*!
 \brief Initializes Modbus registers
 */
void initModbusRegisters(void)
{ 
    /*  Discrete outputs */
    for (int i = 0; i < NUMBER_OF_DOUTS;) {
        DiscreteOutputs[i++] = HIGH;
        DiscreteOutputs[i++] = LOW;
    }
    /*  Discrete inputs */
    for (int i = 0; i < NUMBER_OF_DINPS;) {
        DiscreteInputs[i++] = HIGH;
        DiscreteInputs[i++] = LOW;
    }
    /* Holding registers */
    for (int i = 0; i < NUMBER_OF_HOLDING_REGS; i++) {
        HoldingRegisters[i] = 0xBEAF;
    }
    /* Input registers */
    for (int i = 0; i < NUMBER_OF_INPUT_REGS; i++) {
        InputRegisters[i] = 0xCAFE;
    }
}

/*!
 \brief Validates data in the Modbus request
 \param [IN] value 1 for validate
 \param [IN] value 2 for validate
 \param [IN] reference value
 */
static bool validateRequestData(uint16_t var1, uint16_t var2, uint16_t ref)
{
    if (var1 > (ref - 1)) {
        ExceptionCode = ILLEGAL_DATA_ADDRESS;
        return false;
    }
    if ((var2 == 0) || (var2 > ref)) {
        ExceptionCode = ILLEGAL_DATA_VALUE;
        return false;
    }
    if ((var1 + var2) > ref) {
        ExceptionCode = ILLEGAL_DATA_VALUE;
        return false;
    }
    return true;
}

/* Modbus function 0x01 Read Coil Status */
static uint8_t readCoilStatus(uint8_t *p_data)
{
    volatile uint8_t value = 0;
    uint16_t byte_counter = 0;
    
    uint16_t start_addr = (p_data[0] << 8) | p_data[1];
    uint16_t n_regs = (p_data[2] << 8) | p_data[3];

    // Validate request data
    if (!validateRequestData(start_addr, n_regs, NUMBER_OF_DOUTS))
    {
        return sizeof(uint8_t);
    }
    
    // Read discrete outputs
    for (int i = 0, j = 0; i < n_regs;) {
        if (DiscreteOutputs[start_addr + i++] == HIGH) {
            value |= (0x01 << j);
        }
        j++;
        if ((j == BITS_IN_BYTE) || (i == n_regs)) {
            p_data[1 + byte_counter++] = value;
            value = 0; j = 0;
        }
    }
    
    p_data[0] = byte_counter; // number of data bytes in response
    return p_data[0] + 1; // size of the data field in response
}

/* Modbus function 0x02 Read Discrete Inputs */
static uint8_t readDiscreteInputs(uint8_t *p_data)
{
    volatile uint8_t value = 0;
    uint16_t byte_counter = 0;
    
    uint16_t start_addr = (p_data[0] << 8) | p_data[1];
    uint16_t n_regs = (p_data[2] << 8) | p_data[3];

    // Validate request data
    if (!validateRequestData(start_addr, n_regs, NUMBER_OF_DINPS))
    {
        return sizeof(uint8_t);
    }
    
    // Read discrete inputs
        for (int i = 0, j = 0; i < n_regs;) {
        if (DiscreteInputs[start_addr + i++] == HIGH) {
            value |= (0x01 << j);
        }
        j++;
        if ((j == BITS_IN_BYTE) || (i == n_regs)) {
            p_data[1 + byte_counter++] = value;
            value = 0; j = 0;
        }
    }
    
    p_data[0] = byte_counter; // number of data bytes in response
    return p_data[0] + 1; // size of the data field in response
}

/* Modbus function 0x03 Read Holding Registers */
static uint8_t readHoldingRegisters(uint8_t *p_data)
{
    uint16_t start_addr = (p_data[0] << 8) | p_data[1];
    uint16_t n_regs = (p_data[2] << 8) | p_data[3];
    
    // Validate request data
    if (!validateRequestData(start_addr, n_regs, NUMBER_OF_HOLDING_REGS))
    {
        return sizeof(uint8_t);
    }
    
    p_data[0] = n_regs * 2; // number of data bytes in response
    n_regs += start_addr;
    
    // Read registers
    for (int i = 0; i < n_regs; i++) {
        p_data[1+i*2] = (uint8_t)(HoldingRegisters[i+start_addr] >> 8); // MSB
        p_data[2+i*2] = (uint8_t)HoldingRegisters[i+start_addr];        // LSB
    }
    
    return p_data[0] + 1; // size of the data field in response
}

/* Modbus function 0x04 Read Input Registers */
static uint8_t readInputRegisters(uint8_t *p_data)
{
    uint16_t start_addr = (p_data[0] << 8) | p_data[1];
    uint16_t n_regs = (p_data[2] << 8) | p_data[3];
    
    // Validate request data
    if (!validateRequestData(start_addr, n_regs, NUMBER_OF_INPUT_REGS))
    {
        return sizeof(uint8_t);
    }
    
    p_data[0] = n_regs * 2; // number of data bytes in response
    n_regs += start_addr;
    
    // Read inputs
    for (int i = 0; i < n_regs; i++) {
        p_data[1+i*2] = (uint8_t)(InputRegisters[i+start_addr] >> 8); // MSB
        p_data[2+i*2] = (uint8_t)InputRegisters[i+start_addr];        // LSB
    }

    return p_data[0] + 1; // size of the data field in response
}

/* Modbus function 0x05 Force Single Coil */
static uint8_t forceSingleCoil(uint8_t *p_data)
{
    uint16_t start_addr = (p_data[0] << 8) | p_data[1];
    uint16_t reg_value = (p_data[2] << 8) | p_data[3];
    
    // Validate request data
    if (!validateRequestData(start_addr, 1, NUMBER_OF_DOUTS))
    {
        return sizeof(uint8_t);
    }
    
    // Set register
    if (reg_value == 0) {
        DiscreteOutputs[start_addr] = LOW;
    }
    else if (reg_value == 0xFF00) {
        DiscreteOutputs[start_addr] = HIGH;
    }
    
    return (sizeof(start_addr) + sizeof(reg_value));
}

/* Modbus function 0x06 Preset Single Register */
static uint8_t presetSingleRegister(uint8_t *p_data)
{
    uint16_t start_addr = (p_data[0] << 8) | p_data[1];
    uint16_t reg_value = (p_data[2] << 8) | p_data[3];
    
    // Validate request data
    if (!validateRequestData(start_addr, 1, NUMBER_OF_HOLDING_REGS))
    {
        return sizeof(uint8_t);
    }
    
    // Set register
    HoldingRegisters[start_addr] = reg_value;
    return (sizeof(start_addr) + sizeof(reg_value));
}

/* Modbus function 0x0F Force Multiple Coils */
static uint8_t forceMultipleCoils(uint8_t *p_data)
{
    uint16_t start_addr = (p_data[0] << 8) | p_data[1];
    uint16_t n_regs = (p_data[2] << 8) | p_data[3];
    uint16_t reg_counter = 0;
    uint8_t  n_bytes = p_data[4];
    
    // Validate request data
    if (!validateRequestData(start_addr, n_regs, NUMBER_OF_DOUTS))
    {
        return sizeof(uint8_t);
    }
    
    // Set registers
    for (int i = 0; i < n_bytes; i++) {
        for (int j = 0; j < BITS_IN_BYTE; j++) {
            if (p_data[5 + i] & (0x01 << j)) {
                DiscreteOutputs[reg_counter + start_addr] = HIGH;
            }
            else {
                DiscreteOutputs[reg_counter + start_addr] = LOW;
            }
            if (reg_counter++ == n_regs) {
                break;
            }
        }
    }
    
    return (sizeof(start_addr) + sizeof(n_regs));
}

/* Modbus function 0x0A Preset Multiple Registers */
static uint8_t presetMultipleRegisters(uint8_t *p_data)
{
    uint16_t start_addr = (p_data[0] << 8) | p_data[1];
    uint16_t n_regs = (p_data[2] << 8) | p_data[3];
    
    // Validate request data
    if (!validateRequestData(start_addr, n_regs, NUMBER_OF_HOLDING_REGS))
    {
        return sizeof(uint8_t);
    }
    
    // Set registers
    for(int i = 0; i < n_regs; i++) {
        HoldingRegisters[i+start_addr] = (p_data[5+i*2] << 8) | p_data[6+i*2];
    }
    
    return (sizeof(start_addr) + sizeof(n_regs));
}

/* Other (not supported) functions */
static uint8_t notSupportedFunction(void)
{
    ExceptionCode = ILLEGAL_FUNCTION;
    return sizeof(uint8_t);
}

/*!
 \brief Parsing the received data packet
 \param [IN] pointer to application data buffer
 */
void parseReceivedPacket(void *prm)
{
    uint16_t check_summ;
    uint16_t response_size = 0;
    AppBuffer_t *mb_buffer = (AppBuffer_t*)prm;
    ExceptionCode = NO_ERROR;

    /* Function code processing */
    switch (mb_buffer->rtu_frame->func)
    {
        case 0x01:
            response_size = readCoilStatus(mb_buffer->rtu_frame->pdu);
            break;
        case 0x02:
            response_size = readDiscreteInputs(mb_buffer->rtu_frame->pdu);
            break;
        case 0x03:
            response_size = readHoldingRegisters(mb_buffer->rtu_frame->pdu);
            break;
        case 0x04:
            response_size = readInputRegisters(mb_buffer->rtu_frame->pdu);
            break;
        case 0x05:
            response_size = forceSingleCoil(mb_buffer->rtu_frame->pdu);
            break;
        case 0x06:
            response_size = presetSingleRegister(mb_buffer->rtu_frame->pdu);
            break;
        case 0x0F:
            response_size = forceMultipleCoils(mb_buffer->rtu_frame->pdu);
            break;
        case 0x10:
            response_size = presetMultipleRegisters(mb_buffer->rtu_frame->pdu);
            break;
        default:
            response_size = notSupportedFunction();
            break;
    }
    /* Check exception */
    if (ExceptionCode != NO_ERROR) {
        mb_buffer->rtu_frame->func |= 0x80; // set error flag
        *mb_buffer->rtu_frame->pdu = ExceptionCode; // set exception code
    }
    /* Send response */
    response_size += HEADER_SIZE; // add frame header size
    check_summ = calcBuffCrc16((uint8_t*)mb_buffer->rtu_frame, response_size);
    memcpy((uint8_t*)mb_buffer->rtu_frame+response_size, &check_summ, CHECKSUM_SIZE);
    response_size += CHECKSUM_SIZE; // add checksum size
    sendResponse((uint32_t)mb_buffer->rtu_frame , (uint32_t)response_size);
    mb_buffer->isBusy = false;
}
//eof

#include "crc16.h"

///*!
// \brief Creates CRC 16 table
// */
//static void init_crc16_tab(void)
//{
//    int i, j;
//    unsigned short crc, c;

//    for(i=0; i<256; i++) {
//        crc = 0;
//        c = (unsigned short)i;
//        for(j=0; j<8; j++) {
//            if ((crc ^ c) & 0x0001)
//                crc = (crc >> 1) ^ POLINOME_16;
//            else
//                crc =   crc >> 1;
//            c = c >> 1;
//        }
//        crc_tab16[i] = crc;
//    }
//}

/*!
 \brief Updates CRC16 based on previous value
 \param [IN] crc - previous CRC16 value
 \param [IN] c - data for calculating CRC16
 \retval new CRC16 value
 */
uint16_t updateCrc16(uint16_t crc, uint8_t c)
{
    uint16_t tmp, short_c;

    short_c = 0x00FF & (uint16_t)c;
    tmp = crc ^ short_c;
    crc = (crc >> 8) ^ crc_tab16[tmp & 0xFF];
    return crc;
}

/*!
 \brief Calculates CRC16 for a byte array
 \param [IN] byte array address
 \param [IN] byte array length
 \retval CRC16 value for byte array
 */
uint16_t calcBuffCrc16(uint8_t *data, uint16_t len)
{
    uint16_t i, crc16 = 0xFFFF;
    
    for(i = 0; i < len; ++i) {
        crc16 = updateCrc16(crc16, *(data+i));
    }
    return crc16;
}
// eof

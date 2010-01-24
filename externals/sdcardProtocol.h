#ifndef __SD_CARD_PROTOCOL_H__
#define __SD_CARD_PROTOCOL_H__

/* Common command set */
#define SD_CMD0_GO_IDLE_STATE            0x00
#define SD_CMD1_SEND_OPCOND              0x01
#define SD_CMD9_SEND_CSD                 0x09
#define SD_CMD10_SEND_CID                0x0a
#define SD_CMD12_STOP_TRANSMISSION       0x0b
#define SD_CMD13_SEND_STATUS             0x0c
#define SD_CMD16_SET_BLOCKLEN            0x10
#define SD_CMD17_READ_BLOCK              0x11
#define SD_CMD18_READ_MULTIPLE_BLOCK     0x12
#define SD_CMD24_WRITE_BLOCK             0x18
#define SD_CMD25_WRITE_MULTIPLE_BLOCK    0x19
#define SD_CMD27_PROGRAM_CSD             0x1b
#define SD_CMD28_SET_WRITE_PROT          0x1c
#define SD_CMD29_CLR_WRITE_PROT          0x1d
#define SD_CMD30_SEND_WRITE_PROT         0x1e
#define SD_CMD32_ERASE_WR_BLK_START_ADDR 0x20
#define SD_CMD33_ERASE_WR_BLK_END_ADDR   0x21
#define SD_CMD38_ERASE                   0x26
#define SD_CMD55_APP_CMD                 0x37
#define SD_CMD56_GEN_CMD                 0x38
#define SD_CMD58_READ_OCR                0x3a
#define SD_CMD59_CRC_ON_OFF              0x3b

#define SD_START_TOKEN    0xFE


/* R1 format responses (ORed together as a bit-field) */
#define SD_R1_NOERROR     0x00
#define SD_R1_IDLE        0x01
#define SD_R1_ERASE       0x02
#define SD_R1_ILLEGAL     0x04
#define SD_R1_CRC_ERR     0x08
#define SD_R1_ERASE_SEQ   0x10
#define SD_R1_ADDR_ERR    0x20
#define SD_R1_PARAM_ERR   0x40

/* R2 format responses - second byte only, first is identical to R1 */
#define SD_R2_LOCKED      0x01
#define SD_R2_WP_FAILED   0x02
#define SD_R2_ERROR       0x04
#define SD_R2_CTRL_ERR    0x08
#define SD_R2_ECC_FAIL    0x10
#define SD_R2_WP_VIOL     0x20
#define SD_R2_ERASE_PARAM 0x40
#define SD_R2_RANGE_ERR   0x80

/* Error mask for debug */
#define SD_CARD_ERROR_INIT_1  0x1100
#define SD_CARD_ERROR_INIT_2  0x1200
#define SD_CARD_ERROR_WRITE_1 0x2100
#define SD_CARD_ERROR_WRITE_2 0x2200
#define SD_CARD_ERROR_WRITE_3 0x2300
#define SD_CARD_ERROR_READ_1  0x3100
#define SD_CARD_ERROR_READ_2  0x3200
#define SD_CARD_ERROR_REGISTER_1  0x4100
#define SD_CARD_ERROR_REGISTER_2  0x4200
#define SD_CARD_ERROR_REGISTER_3  0x4300

/* uart communication for debug */
#define SD_CARD_COM_READ_CID   0x01
#define SD_CARD_COM_READ_CSD   0x02
#define SD_CARD_COM_READ_BLOCK 0x10


#endif

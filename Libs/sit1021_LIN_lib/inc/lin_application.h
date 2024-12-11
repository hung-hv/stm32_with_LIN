// lin_application.h
#ifndef LIN_APPLICATION_H
#define LIN_APPLICATION_H

#include <stdint.h>
#include <stdio.h>

#define LIN_MAX_DATA 256  // Maximum data length for a LIN frame

typedef struct {
    uint8_t id;               // Frame ID
    uint8_t data_len;         // Length of the data
    uint8_t* data; // Data array
} LIN_FRAME_t;

//--------------------------------------------------------------
// error messages
//--------------------------------------------------------------
typedef enum {
  LIN_OK  = 0,   // no error
  LIN_WRONG_LEN, // wrong number of data
  LIN_RX_EMPTY,  // no frame received
  LIN_WRONG_CRC  // Checksum wrong
}LIN_ERR_t;

void packDataToLINFrame(LIN_FRAME_t *frame, uint8_t id_set, uint8_t *data, uint8_t len);


#endif /* SIT1021_LIN_H */

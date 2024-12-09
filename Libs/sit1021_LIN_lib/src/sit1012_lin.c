#include "sit1021_lin.h"

// Global variables
static UART_HandleTypeDef *lin_huart;

//--------------------------------------------------------------
// Global variables
//--------------------------------------------------------------
LIN_MASTER_t LIN_MASTER;
LIN_FRAME_t LIN_FRAME;
uint8_t btn = 1;

uint8_t checker_1 = 0;
uint8_t checker_2 = 0;
uint8_t checker_3 = 0;

extern uint8_t flag_read_pin;

void SIT1021_Init(UART_HandleTypeDef *huart) {
    lin_huart = huart;
}

LIN_ERR_t SIT1021_SendData(LIN_FRAME_t *frame, UART_HandleTypeDef *huart) {
    uint8_t checksum = 0;
    uint8_t n = 0;
    uint8_t frame_id = 0;
    uint8_t tx_data = 0;

    // Check the length
    if ((frame->data_len < 1) || (frame->data_len > LIN_MAX_DATA)) {
        return LIN_WRONG_LEN;
    }

    // Calculate checksum
    checksum = p_LIN_makeChecksum(frame);

    // Send Break Field
    HAL_LIN_SendBreak(huart);

    // Send Sync Field
    uint8_t sync_byte = LIN_SYNC_DATA;
    HAL_UART_Transmit(huart, &sync_byte, 1, 100);

    // Send ID Field
    frame_id = frame->frame_id;
    HAL_UART_Transmit(huart, &frame_id, 1, 100);

    // Send Data Field
    for (n = 0; n < frame->data_len; n++) {
        tx_data = frame->data[n];
        HAL_UART_Transmit(huart, &tx_data, 1, 100);
    }

    // Send CRC Field
    HAL_UART_Transmit(huart, &checksum, sizeof(checksum), 100);

    return LIN_OK;
}

LIN_ERR_t SIT1021_ReceiveData(LIN_FRAME_t *frame, UART_HandleTypeDef *huart) {
    uint32_t rx_timeout = 0;
    uint8_t checksum = 0;
    uint8_t n = 0;

    // Check the length
    if ((frame->data_len < 1) || (frame->data_len > LIN_MAX_DATA)) {
        return LIN_WRONG_LEN;
    }

    // Send Break Field
    HAL_LIN_SendBreak(huart);

    // Send Sync Field
    HAL_UART_Transmit(huart, (uint8_t*)LIN_SYNC_DATA, 1, 100);

    // Send ID Field
    HAL_UART_Transmit(huart, &(frame->frame_id), sizeof(frame->frame_id), 100);

    // Prepare master
    LIN_MASTER.mode = RECEIVE_DATA;
    LIN_MASTER.data_ptr = 0;
    LIN_MASTER.crc = 0;

    LIN_FRAME.data_len = frame->data_len;
    LIN_FRAME.data[0] = 0;

    // Wait until frame is received or timeout
    rx_timeout = 0;
    n = 0;
    do {
        rx_timeout++;
        if (rx_timeout > LIN_RX_TIMEOUT_CNT) {
            break;
        }
        if (LIN_MASTER.data_ptr != n) {
            n = LIN_MASTER.data_ptr;
            rx_timeout = 0;
        }
    } while (LIN_MASTER.mode != SEND_DATA);

    if (LIN_MASTER.mode != SEND_DATA) {
        LIN_MASTER.mode = SEND_DATA;
        delay_us(LIN_INTER_FRAME_DELAY);
        return LIN_RX_EMPTY;
    }

    // Copy received data
    for (n = 0; n < frame->data_len; n++) {
        frame->data[n] = LIN_FRAME.data[n];
    }

    // Calculate checksum
    checksum = p_LIN_makeChecksum(frame);

    if (LIN_MASTER.crc != checksum) {
        delay_us(LIN_INTER_FRAME_DELAY);
        return LIN_WRONG_CRC;
    }

    delay_us(LIN_INTER_FRAME_DELAY);
    return LIN_OK;
}

LIN_ERR_t SIT1021_ReceiveData(LIN_FRAME_t *frame, UART_HandleTypeDef *huart) {
    uint8_t checksum = 0;
    uint8_t n = 0;
    uint8_t rx_data = 0;

    // Receive Sync Field
    uint8_t sync_byte;
    HAL_UART_Receive(huart, &sync_byte, 1, HAL_MAX_DELAY);
    if (sync_byte != LIN_SYNC_DATA) {
        return LIN_SYNC_ERROR;
    }

    // Receive PID
    uint8_t pid;
    HAL_UART_Receive(huart, &pid, 1, HAL_MAX_DELAY);
    frame->id = pid & 0x3F;

    // Receive Data
    for (n = 0; n < frame->data_len; n++) {
        HAL_UART_Receive(huart, &frame->data[n], 1, HAL_MAX_DELAY);
    }

    // Receive Checksum
    HAL_UART_Receive(huart, &checksum, 1, HAL_MAX_DELAY);

    // Verify Checksum
    if (checksum != p_LIN_makeChecksum(frame)) {
        return LIN_CHECKSUM_ERROR;
    }

    return LIN_OK;
}


uint8_t p_LIN_makeChecksum(LIN_FRAME_t *frame) {
    uint8_t ret_wert = 0;
    uint8_t n = 0;
    uint16_t dummy = 0;

    dummy = 0;
    for (n = 0; n < frame->data_len; n++) {
        dummy += frame->data[n];
        if (dummy > 0xFF) {
            dummy -= 0xFF;
        }
    }
    ret_wert = (uint8_t)(dummy);
    ret_wert ^= 0xFF;

    return ret_wert;
}

void p_LIN_wait_us(uint32_t n) {
    volatile uint32_t p, t;
    for (p = 0; p < n; p++) {
        for (t = 0; t < 15; t++);
    }
}

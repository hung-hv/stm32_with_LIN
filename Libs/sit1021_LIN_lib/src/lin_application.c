#include "lin_application.h"


#define SET_FRAME_ID(frame, id_set) ((frame)->id = (id_set))

//uint8_t collectData(uint8_t *data, uint8_t len) {
//    // Collect data (this is just an example)
//    for (uint8_t i = 0; i < len; i++) {
//        data[i] = i;
//    }
//}

LIN_ERR_t packDataToLINFrame(LIN_FRAME_t *frame, uint8_t id_set, uint8_t *data, uint8_t len) {
	if (len > LIN_MAX_DATA) {
		/*TODO: return length error*/
		return LIN_WRONG_LEN;
	}
    frame->id = id_set;  /*set id*/
    frame->data_len = len;
    /*TODO: check if data len > 8 -> split to multiple data frame*/
    for (uint8_t i = 0; i < len; i++) {
        frame->data[i] = data[i];
    }
    /*TODO: return LIN_OK here*/
    return LIN_OK;
}

void applicationTask(void) {
	/*TODO: some application in here*/
}

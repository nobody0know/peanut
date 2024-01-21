//
// Created by lc201 on 2024/1/21.
//

#ifndef PEANUT_BSP_CAN_H
#define PEANUT_BSP_CAN_H
#include "main.h"
#include "fdcan.h"
void init_can_filter(FDCAN_HandleTypeDef * hfdcanx);

typedef struct {
        FDCAN_RxHeaderTypeDef rx_header;
        uint8_t rx_date[8];
}CAN_MSG;
#endif //PEANUT_BSP_CAN_H

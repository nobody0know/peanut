//
// Created by lc201 on 2024/1/21.
//

#include "bsp_can.h"
#include "bsp_motor.h"

extern chassis_motor_t motor1; //test

//hfdcanx eg.hfdcan1/2/3
void init_can_filter(FDCAN_HandleTypeDef * hfdcanx)
{
    FDCAN_FilterTypeDef sFilterConfig;
    sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
    sFilterConfig.FilterID1 = 0x000;
    sFilterConfig.FilterID2 = 0x000;
    sFilterConfig.FilterIndex = 0;
    sFilterConfig.FilterType = FDCAN_FILTER_RANGE;
    sFilterConfig.IdType = FDCAN_STANDARD_ID;
    if(HAL_FDCAN_ConfigFilter(hfdcanx, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    if((HAL_FDCAN_ConfigFilter(hfdcanx,&sFilterConfig)!=HAL_OK) != HAL_OK)
    {
        Error_Handler();
    }

    if(HAL_FDCAN_Start(hfdcanx) != HAL_OK)
    {
        Error_Handler();
    }

    /* Activate Rx FIFO 0 new message notification on both FDCAN instances */
    if(HAL_FDCAN_ActivateNotification(hfdcanx, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_FDCAN_Start(hfdcanx);

}

//CAN 收发器接的是5V，需要USB或者XT30供电，stlink只供电芯片的3.3V
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
    static CAN_MSG can_msg;
    if(HAL_FDCAN_GetRxMessage(hfdcan,FDCAN_RX_FIFO0,&can_msg.rx_header,can_msg.rx_date)==HAL_OK)
    {
        if(hfdcan == &hfdcan1)
        {
            chassis_motor_decode(&motor1,CAN_1,can_msg.rx_header.Identifier,can_msg.rx_date);
        }
        else if(hfdcan == &hfdcan2)
        {
            chassis_motor_decode(&motor1,CAN_1,can_msg.rx_header.Identifier,can_msg.rx_date);
        }
        else if(hfdcan == &hfdcan3)
        {
            chassis_motor_decode(&motor1,CAN_1,can_msg.rx_header.Identifier,can_msg.rx_date);
        }
        else
        {
            return;
        }
    }
    else
    {
        return;
    }

}
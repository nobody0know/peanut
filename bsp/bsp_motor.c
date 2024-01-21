//
// Created by lc201 on 2024/1/21.
//

#include "bsp_motor.h"
#include <string.h>
chassis_motor_t motor1; //test
xiaomi_motor_t xmotor1; //test
/**
  * @brief          通过can1/can2，按id以0x201～0x204（CMD_ID=0x200）或0x205～0x208（CMD_ID=0x1FF）的 顺 序 发送电机控制数据
  * @param[in]      选择发送总线为CAN1/CAN2/CAN3/CANX..... 输入hfdcan 1/2/3/x...
  * @param[in]      选择所发送的标识符
  * @param[in]      给id为 0x201/0x205 的电机发送控制数据
  * @param[in]      给id为 0x202/0x206 的电机发送控制数据
  * @param[in]      给id为 0x203/0x207 的电机发送控制数据
  * @param[in]      给id为 0x204/0x208 的电机发送控制数据
  * @retval         返回空
  */
void can_send_dji_motor(FDCAN_HandleTypeDef * hfdcan, DJI_MOTOR_ID CMD_ID, int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4)
{
    uint32_t send_mail_box;
    FDCAN_TxHeaderTypeDef tx_message;
    uint8_t can_send_data[8];
    //Specifies the identifier. This parameter must be a number between:
    //– 0 and 0x7FF, if IdType is FDCAN_STANDARD_ID
    //– 0 and 0x1FFFFFFF, if IdType is FDCAN_EXTENDED_ID
    tx_message.Identifier = CMD_ID;
    //Specifies the identifier type for the message that will be transmitted.
    tx_message.IdType = FDCAN_STANDARD_ID;
    //Specifies the frame type of the message that will be transmitted. T
    tx_message.TxFrameType = FDCAN_DATA_FRAME;
    //Specifies the length of the frame that will be transmitted.
    tx_message.DataLength = FDCAN_DLC_BYTES_8;
    //Specifies the error state indicator.
    tx_message.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    //Specifies whether the Tx frame will be transmitted with or without bit rate switching.
    tx_message.BitRateSwitch = FDCAN_BRS_OFF;
    //Specifies whether the Tx frame will be transmitted in classic or FD format.
    tx_message.FDFormat = FDCAN_CLASSIC_CAN;
    //Specifies the event FIFO control.
    tx_message.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    //Specifies the message marker to be copied into Tx Event FIFO element for identification of Tx message
    //status. This parameter must be a number between 0 and 0xFF
    tx_message.MessageMarker = 0x0;

    can_send_data[0] = motor1 >> 8;
    can_send_data[1] = motor1;
    can_send_data[2] = motor2 >> 8;
    can_send_data[3] = motor2;
    can_send_data[4] = motor3 >> 8;
    can_send_data[5] = motor3;
    can_send_data[6] = motor4 >> 8;
    can_send_data[7] = motor4;

    HAL_FDCAN_AddMessageToTxFifoQ(hfdcan,&tx_message,can_send_data);
}
/**
 * 小米电机控制程序
 * @param xiaomi_motor 小米电机结构体
 * @param hfdcan 使用哪路can hfdcan1/2/3/....
 * @param motor_id 小米电机id
 * @param mode 控制模式 1（使能） 2（失能） 3（设置电机控制模式） 4（设置转速rad/s）
 * @param speed 设置转速时此参数才有用
 */
void can_send_xiaomi_motor(xiaomi_motor_t *xiaomi_motor,FDCAN_HandleTypeDef * hfdcan,uint8_t motor_id,uint8_t mode,float speed)
{
    uint32_t send_mail_box;
    FDCAN_TxHeaderTypeDef tx_message;
    uint8_t can_send_data[8];
    switch (mode) {
        case XIAOMI_MOTOR_START:
            xiaomi_motor->mode = 3;
            break;
        case XIAOMI_MOTOR_STOP:
            xiaomi_motor->mode = 4;
            break;
        case XIAOMI_MOTOR_SET_SPEED_MODE:
        case XIAOMI_MOTOR_SET_SPEED:
            xiaomi_motor->mode = 0x12;
            break;
    }

    xiaomi_motor->id = motor_id;
    xiaomi_motor->res = 0;
    xiaomi_motor->data = 0x10;
    uint32_t * motor_msg2uint32;
    motor_msg2uint32 = (uint32_t *) xiaomi_motor;//把小米电机的自定义信息结构体通过指针强制作为uint32_t去访问来赋值
    //0 and 0x1FFFFFFF, if IdType is FDCAN_EXTENDED_ID
    tx_message.Identifier = *motor_msg2uint32;;
    tx_message.IdType = FDCAN_EXTENDED_ID;
    tx_message.TxFrameType = FDCAN_DATA_FRAME;
    tx_message.DataLength = FDCAN_DLC_BYTES_8;
    tx_message.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    tx_message.BitRateSwitch = FDCAN_BRS_OFF;
    tx_message.FDFormat = FDCAN_CLASSIC_CAN;
    tx_message.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    tx_message.MessageMarker = 0x0;

    for(int i=0;i<8;i++)
    {
        can_send_data[i] = 0;
    }
    switch (mode) {
        case XIAOMI_MOTOR_SET_SPEED_MODE:
        {
            uint16_t index = 0x7005;
            uint8_t run_mode = 2;
            memcpy(&can_send_data[0],&index,2);
            memcpy(&can_send_data[4],&run_mode, 1);
        }
            break;
        case XIAOMI_MOTOR_SET_SPEED:
        {
            uint16_t index = 0x700A;
            memcpy(&can_send_data[0],&index,2);
            memcpy(&can_send_data[4],&speed,4);
        }
            break;
    }

    HAL_FDCAN_AddMessageToTxFifoQ(hfdcan,&tx_message,can_send_data);

}
/*!
 *
 * @brief 大疆C610/C620/GM6020信息解码
 * @param motor 电机数据结构体
 * @param data  can消息报文
 * @retval none
 */
void dji_motor_decode(motor_measure_t *motor,const uint8_t *data)
{
    motor->last_ecd = motor->ecd;
    motor->ecd = (uint16_t)(data[0]<<8 | data[1]);
    motor->speed_rpm = (int16_t)(data[2]<<8 | data[3]);
    motor->given_current = (int16_t)(data[4]<<8 | data[5]);
    motor->temperate = data[6];
}

/*!
 *
 * @brief 维特智能单圈绝对值编码器信息解码
 * @param encoder 电机数据结构体
 * @param data  can消息报文
 * @retval none
 */
void encoder_decode(encoder_t *encoder,const uint8_t *data)
{
    if(data[0] == 0x55&&data[1]==0x55)
    {

        encoder->real_angle= (float )(data[3]<<8 | data[2]) * 360 / 32768;//角度 = 角度寄存器数值*360/32768
        encoder->angle = encoder->real_angle - encoder->offset_angle;
        if(encoder->angle >= 180)
            encoder->angle -=360;
        else if(encoder->angle<=-180)
            encoder->angle +=360;
        if(encoder->angle>=180 || encoder->angle<=-180)
            encoder->angle = encoder->last_angle;
        encoder->last_angle = encoder->angle;
        encoder->speed_rpm = (int16_t)(data[5]<<8 | data[4]);
        encoder->angular_v = (float )(data[7]<<8 | data[6]) * 360 /32768;//角速度 = 角速度寄存器数值*360/32768
    }
    else if(data[0]==0x55&&data[1]==0x56)
    {
        encoder->temperate = (float )(data[3]<<8 |data[2])/100;
    }
}

void dji_motor_round_count(motor_measure_t *motor)
{
    if(motor->ecd - motor->last_ecd>4192){
        motor->round_count--;
    }
    else if(motor->ecd-motor->last_ecd< -4192)
    {
        motor->round_count++;
    }
    motor->total_ecd = motor->round_count*8192 + (motor->ecd - motor->offset_ecd);
}

fp32 motor_ecd_to_angle_change(uint16_t ecd, uint16_t offset_ecd) {
    int32_t tmp = 0;
    if (offset_ecd >= 4096) {
        if (ecd > offset_ecd - 4096) {
            tmp = ecd - offset_ecd;
        } else {
            tmp = ecd + 8192 - offset_ecd;
        }
    } else {
        if (ecd > offset_ecd + 4096) {
            tmp = ecd - 8192 - offset_ecd;
        } else {
            tmp = ecd - offset_ecd;
        }
    }
    return (fp32) tmp / 8192.f * 360;
}

/*!
 *
 * @brief 底盘信息解码
 * @param motor 电机数据结构体
 * @param can_id can_id
 * @param can_msg  can消息报文
 * @retval none
 */
void chassis_motor_decode(chassis_motor_t *motor,uint8_t can_type,uint32_t can_id,uint8_t * can_msg)
{
    if(can_type == CAN_1)
    {
        dji_motor_decode(&motor->motor_info,can_msg);
        switch (can_id) {

            default: {
                break;
            }
        }
    }
    else if(can_type == CAN_2)
    {

    }
}
/*!
 *
 * @brief 云台信息解码
 * @param motor 电机数据结构体
 * @param can_id can_id
 * @param can_msg  can消息报文
 * @retval none
 */
void gimbal_motor_decode(gimbal_motor_t *motor,uint8_t can_type,uint32_t can_id,uint8_t * can_msg)
{
    if(can_type == CAN_1)
    {
        switch (can_id) {

            case CAN_GIMBAL_XIAOMI_YAW:{
                //暂时不需要小米电机的数据反馈，云台姿态由编码器反馈
            }
                break;
            default: {
                break;
            }
        }
    }
}

void yaw_encoder_decode(encoder_t *encoder,uint8_t can_type,uint32_t can_id,uint8_t * can_msg)
{
    if(can_type == CAN_1)
    {
        switch (can_id) {
            case 0x50:{
                encoder_decode(encoder,can_msg);
            }
                break;
            default: {
                break;
            }
        }
    }
}




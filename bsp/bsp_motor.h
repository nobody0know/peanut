//
// Created by lc201 on 2024/1/21.
//

#ifndef PEANUT_BSP_MOTOR_H
#define PEANUT_BSP_MOTOR_H


#include "struct_typedef.h"

typedef enum {
    CAN_1,
    CAN_2,
}CAN_TYPE;

typedef enum{
    CAN_DJI_MOTOR_0x200_ID = 0x200,//C620/C610 id=1~4 (0x201~0x204)
    CAN_DJI_MOTOR_0x1FF_ID = 0x1FF,//C620/C610 id=5~8 (0x205~0x208);GM6020 id=1~4 (0x205~0x208)
    CAN_DJI_MOTOR_0x2FF_ID = 0x2FF,//GM6020 id=5~7 (0x209~0x20B)
}DJI_MOTOR_ID;

typedef enum{
    XIAOMI_MOTOR_START = 1,
    XIAOMI_MOTOR_STOP = 2,
    XIAOMI_MOTOR_SET_SPEED_MODE = 3,
    XIAOMI_MOTOR_SET_SPEED = 4
}XIAOMI_MOTOR_MODE;

typedef enum{
    //0X200对应的电机ID(CAN1)
    CAN_CHASSIS_3508_MOTOR_RF=0x202,
    CAN_CHASSIS_3508_MOTOR_LF=0x201,
    CAN_CHASSIS_3508_MOTOR_LB=0x204,
    CAN_CHASSIS_3508_MOTOR_RB=0x203,

    //0X1FF对应的电机ID(CAN1)
    CAN_GIMBAL_6020_YAW=0x205,
    CAN_GIMBAL_XIAOMI_YAW = 0x64,

    //0X200对应的电机ID(CAN2)

    //0X1FF对应的电机ID(CAN2)

}CAN_ID;

typedef struct
{
    uint16_t ecd;
    int16_t speed_rpm;
    int16_t given_current;
    uint8_t temperate;
    uint16_t last_ecd;

    int32_t total_ecd;   //电机旋转的总编码器数值
    int32_t round_count;
    uint16_t offset_ecd;//电机的校准编码值
} motor_measure_t;

typedef struct
{
    float angle; //减去中值的角度 360
    float speed_rpm;//转速
    float angular_v;//角速度
    float temperate;
    float offset_angle;
    float real_angle;//原始角度
    float last_angle;
} encoder_t;

typedef struct
{
    motor_measure_t motor_info;

    fp32 rpm_set;
    int16_t give_current;
}chassis_motor_t;

typedef struct
{
    motor_measure_t motor_info;

    fp32 relative_angle_get;
    fp32 relative_angle_set; //°
    fp32 relative_gyro_get;

    fp32 gyro_set;  //转速设置
    int16_t give_current; //最终电流值
}gimbal_motor_t;

typedef struct
{
    motor_measure_t motor_info;

    fp32 rpm_set;
    int16_t give_current;

}launcher_motor_t;

typedef struct
{
    uint32_t id:8;
    uint32_t data:16;
    uint32_t mode:5;
    uint32_t res:3;
}xiaomi_motor_t;


extern void chassis_motor_decode(chassis_motor_t *motor,uint8_t can_type,uint32_t can_id,uint8_t * can_msg);
extern fp32 motor_ecd_to_angle_change(uint16_t ecd,uint16_t offset_ecd);
extern void gimbal_motor_decode(gimbal_motor_t *motor,uint8_t can_type,uint32_t can_id,uint8_t * can_msg);
extern void yaw_encoder_decode(encoder_t *encoder,uint8_t can_type,uint32_t can_id,uint8_t * can_msg);
extern void can_send_dji_motor(FDCAN_HandleTypeDef * hfdcan, DJI_MOTOR_ID CMD_ID, int16_t motor1, int16_t motor2, int16_t motor3, int16_t motor4);
extern void motor_init(chassis_motor_t *chassis_motor);
extern void can_send_xiaomi_motor(xiaomi_motor_t *xiaomi_motor,FDCAN_HandleTypeDef * hfdcan,uint8_t motor_id,uint8_t mode,float speed);



#endif //PEANUT_BSP_MOTOR_H

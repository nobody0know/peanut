//
// Created by lc201 on 2024/1/21.
//

#include "bsp_lsm6dsl.h"
/*
 ******************************************************************************
 * @file    read_data_polling.c
 * @author  Sensors Software Solution Team
 * @brief   This file show how to get data from sensor.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/*
 * This example was developed using the following STMicroelectronics
 * evaluation boards:
 *
 * - STEVAL_MKI109V3 + STEVAL-MKI178V2
 * - NUCLEO_F411RE + X-NUCLEO-IKS01A2
 * - DISCOVERY_SPC584B + STEVAL-MKI178V2
 *
 * Used interfaces:
 *
 * STEVAL_MKI109V3    - Host side:   USB (Virtual COM)
 *                    - Sensor side: SPI(Default) / I2C(supported)
 *
 * NUCLEO_STM32F411RE - Host side: UART(COM) to USB bridge
 *                    - I2C(Default) / SPI(supported)
 *
 * DISCOVERY_SPC584B  - Host side: UART(COM) to USB bridge
 *                    - Sensor side: I2C(Default) / SPI(supported)
 *
 * If you need to run this example on a different hardware platform a
 * modification of the functions: `platform_write`, `platform_read`,
 * `tx_com` and 'platform_init' is required.
 *
 */

/* STMicroelectronics evaluation boards definition
 *
 * Please uncomment ONLY the evaluation boards in use.
 * If a different hardware is used please comment all
 * following target board and redefine yours.
 */

//#define STEVAL_MKI109V3  /* little endian */
//#define NUCLEO_F411RE    /* little endian */
//#define SPC584B_DIS      /* big endian */

/* ATTENTION: By default the driver is little endian. If you need switch
 *            to big endian please see "Endianness definitions" in the
 *            header file of the driver (_reg.h).
 */
#define SENSOR_BUS hi2c3


/* Includes ------------------------------------------------------------------*/
#include "lsm6dsl_reg.h"
#include <string.h>
#include <stdio.h>

#include "stm32g4xx_hal.h"
#include "usart.h"
#include "gpio.h"
#include "i2c.h"


/* Private macro -------------------------------------------------------------*/
#define TX_BUF_DIM          1000
#define    BOOT_TIME          15 //ms

/* Private variables ---------------------------------------------------------*/
static int16_t data_raw_acceleration[3];
static int16_t data_raw_angular_rate[3];
static int16_t data_raw_temperature;
static float acceleration_mg[3];
static float angular_rate_mdps[3];
static float temperature_degC;
static uint8_t whoamI, rst;
static uint8_t tx_buffer[TX_BUF_DIM];

/* Extern variables ----------------------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/*
 *   WARNING:
 *   Functions declare in this section are defined at the end of this file
 *   and are strictly related to the hardware platform used.
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
static void platform_delay(uint32_t ms);


/* Main Example --------------------------------------------------------------*/
void lsm6dsl_read_data_polling(void)
{
    /* Initialize mems driver interface */
    stmdev_ctx_t dev_ctx;
    dev_ctx.write_reg = platform_write;
    dev_ctx.read_reg = platform_read;
    dev_ctx.handle = &SENSOR_BUS;

    /* Wait sensor boot time */
    platform_delay(BOOT_TIME);
    /* Check device ID */
    whoamI = 0;
    lsm6dsl_device_id_get(&dev_ctx, &whoamI);

    if ( whoamI != LSM6DSL_ID )
        while (1); /*manage here device not found */

    /* Restore default configuration */
    lsm6dsl_reset_set(&dev_ctx, PROPERTY_ENABLE);

    do {
        lsm6dsl_reset_get(&dev_ctx, &rst);
    } while (rst);

    /* Enable Block Data Update */
    lsm6dsl_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
    /* Set Output Data Rate */
    lsm6dsl_xl_data_rate_set(&dev_ctx, LSM6DSL_XL_ODR_1k66Hz);//修改陀螺仪的数据回报率
    lsm6dsl_gy_data_rate_set(&dev_ctx, LSM6DSL_GY_ODR_1k66Hz);
    /* Set full scale */
    lsm6dsl_xl_full_scale_set(&dev_ctx, LSM6DSL_2g);
    lsm6dsl_gy_full_scale_set(&dev_ctx, LSM6DSL_2000dps);
    /* Configure filtering chain(No aux interface) */
    /* Accelerometer - analog filter */
    lsm6dsl_xl_filter_analog_set(&dev_ctx, LSM6DSL_XL_ANA_BW_400Hz);
    /* Accelerometer - LPF1 path ( LPF2 not used )*/
    //lsm6dsl_xl_lp1_bandwidth_set(&dev_ctx, LSM6DSL_XL_LP1_ODR_DIV_4);
    /* Accelerometer - LPF1 + LPF2 path */
    lsm6dsl_xl_lp2_bandwidth_set(&dev_ctx,
                                 LSM6DSL_XL_LOW_NOISE_LP_ODR_DIV_100);
    /* Accelerometer - High Pass / Slope path */
    //lsm6dsl_xl_reference_mode_set(&dev_ctx, PROPERTY_DISABLE);
    //lsm6dsl_xl_hp_bandwidth_set(&dev_ctx, LSM6DSL_XL_HP_ODR_DIV_100);
    /* Gyroscope - filtering chain */
    lsm6dsl_gy_band_pass_set(&dev_ctx, LSM6DSL_HP_260mHz_LP1_STRONG);

    /* Read samples in polling mode (no int) */
    while (1) {
        /* Read output only if new value is available */
        lsm6dsl_reg_t reg;
        lsm6dsl_status_reg_get(&dev_ctx, &reg.status_reg);

        if (reg.status_reg.xlda) {
            /* Read magnetic field data */
            memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
            lsm6dsl_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
            acceleration_mg[0] = lsm6dsl_from_fs2g_to_mg(
                    data_raw_acceleration[0]);
            acceleration_mg[1] = lsm6dsl_from_fs2g_to_mg(
                    data_raw_acceleration[1]);
            acceleration_mg[2] = lsm6dsl_from_fs2g_to_mg(
                    data_raw_acceleration[2]);
            sprintf((char *)tx_buffer,
                    "Acceleration [mg]:%4.2f\t%4.2f\t%4.2f\r\n",
                    acceleration_mg[0], acceleration_mg[1], acceleration_mg[2]);

        }

        if (reg.status_reg.gda) {
            /* Read magnetic field data */
            memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
            lsm6dsl_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate);
            angular_rate_mdps[0] = lsm6dsl_from_fs2000dps_to_mdps(
                    data_raw_angular_rate[0]);
            angular_rate_mdps[1] = lsm6dsl_from_fs2000dps_to_mdps(
                    data_raw_angular_rate[1]);
            angular_rate_mdps[2] = lsm6dsl_from_fs2000dps_to_mdps(
                    data_raw_angular_rate[2]);
            sprintf((char *)tx_buffer,
                    "Angular rate [mdps]:%4.2f\t%4.2f\t%4.2f\r\n",
                    angular_rate_mdps[0], angular_rate_mdps[1], angular_rate_mdps[2]);

        }

        if (reg.status_reg.tda) {
            /* Read temperature data */
            memset(&data_raw_temperature, 0x00, sizeof(int16_t));
            lsm6dsl_temperature_raw_get(&dev_ctx, &data_raw_temperature);
            temperature_degC = lsm6dsl_from_lsb_to_celsius(
                    data_raw_temperature );
            sprintf((char *)tx_buffer, "Temperature [degC]:%6.2f\r\n",
                    temperature_degC );

        }
    }
}

/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp, uint16_t len)
{
    HAL_I2C_Mem_Write(handle, LSM6DSL_I2C_ADD_L, reg,
                    I2C_MEMADD_SIZE_8BIT, (uint8_t*) bufp, len, 1000);

    return 0;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp, uint16_t len)
{
    HAL_I2C_Mem_Read(handle, LSM6DSL_I2C_ADD_L, reg,
                   I2C_MEMADD_SIZE_8BIT, bufp, len, 1000);

    return 0;
}



/*
 * @brief  platform specific delay (platform dependent)
 *
 * @param  ms        delay in ms
 *
 */
static void platform_delay(uint32_t ms)
{
    HAL_Delay(ms);
}


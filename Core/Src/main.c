/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "app_fatfs.h"
#include "fdcan.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "bsp_can.h"
#include "bsp_motor.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t rx_buffer_uart1[256];
uint8_t rx_buffer_uart4[256];
uint8_t rx_buffer_uart5[256];
char SPIFLASHPath[4];             /* 串行Flash逻辑设备路径 */
char SDPath[4];                   /* SD卡逻辑设备路径 */

FATFS fs;													/* FatFs文件系统对象 */
FIL file;													/* 文件对象 */
FRESULT f_res;                    /* 文件操作结果 */
UINT fnum;            					  /* 文件成功读写数量 */
BYTE ReadBuffer[1024]={0};        /* 读缓冲区 */
BYTE WriteBuffer[]= "新建文件系统测试文件\n";/* 写缓冲区*/


extern void lsm6dsl_read_data_polling(void);
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FDCAN1_Init();
  MX_FDCAN2_Init();
  MX_FDCAN3_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_UART4_Init();
  MX_UART5_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
  MX_USB_PCD_Init();
  MX_TIM2_Init();
  MX_I2C3_Init();
  MX_TIM8_Init();
  MX_TIM5_Init();
  if (MX_FATFS_Init() != APP_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN 2 */
    init_can_filter(&hfdcan1);
    init_can_filter(&hfdcan2);
    init_can_filter(&hfdcan3);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1,rx_buffer_uart1,sizeof(rx_buffer_uart1));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart4,rx_buffer_uart4,sizeof(rx_buffer_uart4));
    HAL_UARTEx_ReceiveToIdle_DMA(&huart5,rx_buffer_uart5,sizeof(rx_buffer_uart5));

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
      /* 注册一个FatFS设备：SD卡 */
      if(FATFS_LinkDriver(&USER_Driver, SDPath) == 0)
      {
          //在SD卡挂载文件系统，文件系统挂载时会对SD卡初始化
          f_res = f_mount(&fs,(TCHAR const*)SDPath,1);
//          printf_fatfs_error(f_res);
          /*----------------------- 格式化测试 ---------------------------*/
          /* 如果没有文件系统就格式化创建创建文件系统 */
          if(f_res == FR_NO_FILESYSTEM)
          {
//              printf("》SD卡还没有文件系统，即将进行格式化...\n");
              /* 格式化 */
              f_res=f_mkfs((TCHAR const*)SDPath,FM_FAT32,0,ReadBuffer,64);

              if(f_res == FR_OK)
              {
//                  printf("》SD卡已成功格式化文件系统。\n");
                  /* 格式化后，先取消挂载 */
                  f_res = f_mount(NULL,(TCHAR const*)SDPath,1);
                  /* 重新挂载	*/
                  f_res = f_mount(&fs,(TCHAR const*)SDPath,1);
              }
              else
              {
//                  printf("《《格式化失败。》》\n");
                  while(1);
              }
          }
          else if(f_res!=FR_OK)
          {
//              printf("！！SD卡挂载文件系统失败。(%d)\n",f_res);
//              printf_fatfs_error(f_res);
              while(1);
          }
          else
          {
//              while(1);
//              printf("》文件系统挂载成功，可以进行读写测试\n");
          }

          /*----------------------- 文件系统测试：写测试 -----------------------------*/
          /* 打开文件，如果文件不存在则创建它 */
//          printf("****** 即将进行文件写入测试... ******\n");
          f_res = f_open(&file, "FatFs_test.txt",FA_CREATE_ALWAYS | FA_WRITE );
          if ( f_res == FR_OK )
          {
//              printf("》打开/创建FatFs读写测试文件.txt文件成功，向文件写入数据。\n");
              /* 将指定存储区内容写入到文件内 */
              f_res=f_write(&file,WriteBuffer,sizeof(WriteBuffer),&fnum);
              if(f_res==FR_OK)
              {
//                  printf("》文件写入成功，写入字节数据：%d\n",fnum);
//                  printf("》向文件写入的数据为：\n%s\n",WriteBuffer);
              }
              else
              {
                  while(1);
//                  printf("！！文件写入失败：(%d)\n",f_res);
              }
              /* 不再读写，关闭文件 */
              f_close(&file);
          }
          else
          {
              while(1);
//              printf("！！打开/创建文件失败。\n");
          }

          /*------------------- 文件系统测试：读测试 ------------------------------------*/
//          printf("****** 即将进行文件读取测试... ******\n");
          f_res = f_open(&file, "FatFs_test.txt", FA_OPEN_EXISTING | FA_READ);
          if(f_res == FR_OK)
          {
//              printf("》打开文件成功。\n");
              f_res = f_read(&file, ReadBuffer, sizeof(ReadBuffer), &fnum);
              if(f_res==FR_OK)
              {
//                  printf("》文件读取成功,读到字节数据：%d\n",fnum);
//                  printf("》读取得的文件数据为：\n%s \n", ReadBuffer);
              }
              else
              {
                  while(1);
//                  printf("！！文件读取失败：(%d)\n",f_res);
              }
          }
          else
          {
              while(1);
//              printf("！！打开文件失败。\n");
          }
          /* 不再读写，关闭文件 */
          f_close(&file);

          /* 不再使用，取消挂载 */
          f_res = f_mount(NULL,(TCHAR const*)SDPath,1);
      }

      /* 注销一个FatFS设备：SD卡 */
      FATFS_UnLinkDriver(SDPath);
//      lsm6dsl_read_data_polling();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV2;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

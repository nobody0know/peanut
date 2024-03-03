//
// Created by lc201 on 2023/12/25.
//

#include "bsp_usart.h"
#include "usart.h"

extern uint8_t rx_buffer_1[1024];
extern uint8_t rx_buffer_2[1024];

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size) {

}

//重写printf
void __io_putchar(uint8_t * ch)
{
    HAL_UART_Transmit(&huart1,ch, sizeof(uint8_t),1000);
}
int _write(int fd, char *pBuffer,int size)
{
    for(int i = 0; i < size; i++)
    {
        __io_putchar((uint8_t *)pBuffer++);
    }
    return size;
}
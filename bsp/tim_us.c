#include "tim_us.h"


void delay_us(uint16_t us)
{
    uint16_t delay=0xffff-us-5;				//65535-5-us，计数初值，由这个数
    										//计数到65535刚好计数us次，也就实
    										//现了nus延时
    HAL_TIM_Base_Start(&htim1);  			//开启定时器
    __HAL_TIM_SetCounter(&htim1,delay);		//设置计数值
    while(delay<0xffff-5)
    {
        delay=__HAL_TIM_GetCounter(&htim1); //获取当前计数值
    }
    HAL_TIM_Base_Stop(&htim1);
}

// SPDX-License-Identifier: GPL-3.0-only
/*
 * Copyright (c) 2008-2023 100askTeam : Dongshan WEI <weidongshan@qq.com> 
 * Discourse:  https://forums.100ask.net
 */
 
/*  Copyright (C) 2008-2023 深圳百问网科技有限公司
 *  All rights reserved
 *
 * 免责声明: 百问网编写的文档, 仅供学员学习使用, 可以转发或引用(请保留作者信息),禁止用于商业用途！
 * 免责声明: 百问网编写的程序, 可以用于商业用途, 但百问网不承担任何后果！
 * 
 * 本程序遵循GPL V3协议, 请遵循协议
 * 百问网学习平台   : https://www.100ask.net
 * 百问网交流社区   : https://forums.100ask.net
 * 百问网官方B站    : https://space.bilibili.com/275908810
 * 本程序所用开发板 : DShanMCU-F103
 * 百问网官方淘宝   : https://100ask.taobao.com
 * 联系我们(E-mail): weidongshan@qq.com
 *
 *          版权所有，盗版必究。
 *  
 * 修改历史     版本号           作者        修改内容
 *-----------------------------------------------------
 * 2023.08.04      v01         百问科技      创建文件
 *-----------------------------------------------------
 */

#include "main.h"
#include "driver_mpu6050.h"
#include <math.h>
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"               // ARM.FreeRTOS::RTOS:Event Groups

QueueHandle_t g_xQueueMPU6050; /* MPU6050队列 */

//****************************************
// 定义MPU6050内部地址
//****************************************
#define	MPU6050_SMPLRT_DIV		0x19  // 陀螺仪采样率，典型值：0x07(125Hz)
#define	MPU6050_CONFIG			0x1A  // 低通滤波频率，典型值：0x06(5Hz)
#define	MPU6050_GYRO_CONFIG		0x1B  // 陀螺仪自检及测量范围，典型值：0x18(不自检，2000deg/s)
#define	MPU6050_ACCEL_CONFIG	0x1C  // 加速计自检、测量范围及高通滤波频率，典型值：0x01(不自检，2G，5Hz)

#define	MPU6050_ACCEL_XOUT_H	0x3B
#define	MPU6050_ACCEL_XOUT_L	0x3C
#define	MPU6050_ACCEL_YOUT_H	0x3D
#define	MPU6050_ACCEL_YOUT_L	0x3E
#define	MPU6050_ACCEL_ZOUT_H	0x3F
#define	MPU6050_ACCEL_ZOUT_L	0x40
#define	MPU6050_TEMP_OUT_H		0x41
#define	MPU6050_TEMP_OUT_L		0x42
#define	MPU6050_GYRO_XOUT_H		0x43
#define	MPU6050_GYRO_XOUT_L		0x44
#define	MPU6050_GYRO_YOUT_H		0x45
#define	MPU6050_GYRO_YOUT_L		0x46
#define	MPU6050_GYRO_ZOUT_H		0x47
#define	MPU6050_GYRO_ZOUT_L		0x48
#define MPU6050_MOT_THR 0x1f
#define MPU6050_MOT_DUR 0x20

#define	MPU6050_INT_PIN_CFG     0x37
#define	MPU6050_INT_ENABLE      0x38

#define	MPU6050_PWR_MGMT_1		0x6B //电源管理，典型值：0x00(正常启用)
#define	MPU6050_PWR_MGMT_2		0x6C
#define	MPU6050_WHO_AM_I		0x75 //IIC地址寄存器(默认数值0x68，只读)

#define MPU6050_I2C_ADDR     0xD0
#define MPU6050_TIMEOUT     500

/* 传感器数据修正值（消除芯片固定误差，根据硬件进行调整） */
#define MPU6050_X_ACCEL_OFFSET	(-64) 
#define MPU6050_Y_ACCEL_OFFSET 	(-30)
#define MPU6050_Z_ACCEL_OFFSET 	(14400) 
#define MPU6050_X_GYRO_OFFSET 	(40)
#define MPU6050_Y_GYRO_OFFSET 	(-7)
#define MPU6050_Z_GYRO_OFFSET 	(-14)

QueueHandle_t g_xQueueMPU6050; /* MPU6050队列 */
static EventGroupHandle_t g_xEventMPU6050;

extern I2C_HandleTypeDef hi2c2;
static I2C_HandleTypeDef *g_pHI2C_MPU6050 = &hi2c2;


/**********************************************************************
 * 函数名称： GetQueueMPU6050
 * 功能描述： 返回MPU6050驱动程序里的队列句柄
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 队列句柄
 * 修改日期：      版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/09/05	     V1.0	  韦东山	      创建
 ***********************************************************************/
QueueHandle_t GetQueueMPU6050(void)
{
	return g_xQueueMPU6050;
}

/**********************************************************************
 * 函数名称： MPU6050_WriteRegister
 * 功能描述： 写MPU6050寄存器
 * 输入参数： reg-寄存器地址, data-要写入的数据
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/03	     V1.0	  韦东山	      创建
 ***********************************************************************/
static int MPU6050_WriteRegister(uint8_t reg, uint8_t data)
{
    uint8_t tmpbuf[2];

    tmpbuf[0] = reg;
    tmpbuf[1] = data;
    
    return HAL_I2C_Master_Transmit(g_pHI2C_MPU6050, MPU6050_I2C_ADDR, tmpbuf, 2, MPU6050_TIMEOUT);
}

/**********************************************************************
 * 函数名称： MPU6050_ReadRegister
 * 功能描述： 读MPU6050寄存器
 * 输入参数： reg-寄存器地址
 * 输出参数： pdata-用来保存读出的数据
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/03	     V1.0	  韦东山	      创建
 ***********************************************************************/
int MPU6050_ReadRegister(uint8_t reg, uint8_t *pdata)
{
	return HAL_I2C_Mem_Read(g_pHI2C_MPU6050, MPU6050_I2C_ADDR, reg, 1, pdata, 1, MPU6050_TIMEOUT);
}

/**********************************************************************
 * 函数名称： MPU6050_Init
 * 功能描述： MPU6050初始化函数,
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/03	     V1.0	  韦东山	      创建
 ***********************************************************************/
int MPU6050_Init(void)
{
#if 0	
	MPU6050_WriteRegister(MPU6050_PWR_MGMT_1, 0x00);	//解除休眠状态
	MPU6050_WriteRegister(MPU6050_PWR_MGMT_2, 0x00);
	MPU6050_WriteRegister(MPU6050_SMPLRT_DIV, 0x09);
	MPU6050_WriteRegister(MPU6050_CONFIG, 0x06);
	MPU6050_WriteRegister(MPU6050_GYRO_CONFIG, 0x18);
	MPU6050_WriteRegister(MPU6050_ACCEL_CONFIG, 0x18);

	/* 参考: https://blog.csdn.net/sjf8888/article/details/97912391 */	
	/* 配置中断引脚 */
	MPU6050_WriteRegister(MPU6050_INT_PIN_CFG, 0);
	
	/* 使能中断 */
	MPU6050_WriteRegister(MPU6050_INT_ENABLE, 0xff);
#else
        MPU6050_WriteRegister(MPU6050_PWR_MGMT_1, 0x80);        //复位
        vTaskDelay(100);
        MPU6050_WriteRegister(MPU6050_PWR_MGMT_1, 0x00);        //解除休眠状态
        MPU6050_WriteRegister(MPU6050_GYRO_CONFIG, 0x0);
        MPU6050_WriteRegister(MPU6050_ACCEL_CONFIG, 0x18);
        
        MPU6050_WriteRegister(MPU6050_SMPLRT_DIV, 0x09);
        MPU6050_WriteRegister(MPU6050_CONFIG, 0x06);
        //

		MPU6050_WriteRegister(MPU6050_MOT_THR, 3);          //设置加速度阈值为74mg
		MPU6050_WriteRegister(MPU6050_MOT_DUR, 20);          //设置加速度检测时间20ms
		MPU6050_WriteRegister(MPU6050_CONFIG,0x04);           //配置外部引脚采样和DLPF数字低通滤波器
		MPU6050_WriteRegister(MPU6050_ACCEL_CONFIG,4);     //加速度传感器量程和高通滤波器配置
        
        /* 参考: https://blog.csdn.net/sjf8888/article/details/97912391 */        
        /* 配置中断引脚 */
        MPU6050_WriteRegister(MPU6050_INT_PIN_CFG, 0x10);
        
        /* 使能中断 */
        MPU6050_WriteRegister(MPU6050_INT_ENABLE, 0x40);	
#endif	
	g_xQueueMPU6050 = xQueueCreate(MPU6050_QUEUE_LEN, sizeof(struct mpu6050_data));
	g_xEventMPU6050 = xEventGroupCreate();
	
	return 0;
}

/**********************************************************************
 * 函数名称： MPU6050_GetID
 * 功能描述： 读取MPU6050 ID
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： -1 - 失败, 其他值 - ID
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/03	     V1.0	  韦东山	      创建
 ***********************************************************************/
int MPU6050_GetID(void)
{
    uint8_t id;
	if(0 == MPU6050_ReadRegister(MPU6050_WHO_AM_I, &id))
        return id;
    else
        return -1;
}



/**********************************************************************
 * 函数名称： MPU6050_ReadData
 * 功能描述： 读取MPU6050数据
 * 输入参数： 无
 * 输出参数： pAccX/pAccY/pAccZ         - 用来保存X/Y/Z轴的加速度
 *            pGyroX/pGyroY/pGyroZ - 用来保存X/Y/Z轴的角速度
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/08/03	     V1.0	  韦东山	      创建
 ***********************************************************************/
int MPU6050_ReadData(int16_t *pAccX, int16_t *pAccY, int16_t *pAccZ, int16_t *pGyroX, int16_t *pGyroY, int16_t *pGyroZ)
{
	uint8_t datal, datah;
    int err = 0;
	
	if(pAccX)
	{
		err |= MPU6050_ReadRegister(MPU6050_ACCEL_XOUT_H, &datah);
		err |= MPU6050_ReadRegister(MPU6050_ACCEL_XOUT_L, &datal);
        *pAccX = (datah << 8) | datal;
	}

	if(pAccY)
	{
		err |= MPU6050_ReadRegister(MPU6050_ACCEL_YOUT_H, &datah);
		err |= MPU6050_ReadRegister(MPU6050_ACCEL_YOUT_L, &datal);
        *pAccY = (datah << 8) | datal;
	}
	
	if(pAccZ)
	{
		err |= MPU6050_ReadRegister(MPU6050_ACCEL_ZOUT_H, &datah);
		err |= MPU6050_ReadRegister(MPU6050_ACCEL_ZOUT_L, &datal);
        *pAccZ = (datah << 8) | datal;
	}


	if(pGyroX)
	{
		err |= MPU6050_ReadRegister(MPU6050_GYRO_XOUT_H, &datah);
		err |= MPU6050_ReadRegister(MPU6050_GYRO_XOUT_L, &datal);
        *pGyroX = (datah << 8) | datal;
	}

	
	if(pGyroY)
	{
		err |= MPU6050_ReadRegister(MPU6050_GYRO_YOUT_H, &datah);
		err |= MPU6050_ReadRegister(MPU6050_GYRO_YOUT_L, &datal);
        *pGyroY = (datah << 8) | datal;
	}
	
	if(pGyroZ)
	{
		err |= MPU6050_ReadRegister(MPU6050_GYRO_ZOUT_H, &datah);
		err |= MPU6050_ReadRegister(MPU6050_GYRO_ZOUT_L, &datal);
        *pGyroZ = (datah << 8) | datal;
	}

    return err;	
}

/**********************************************************************
 * 函数名称： MPU6050_ParseData
 * 功能描述： 解析MPU6050数据
 * 输入参数： AccX/AccY/AccZ/GyroX/GyroY/GyroZ
 *            X/Y/Z轴的加速度,X/Y/Z轴的角速度
 * 输出参数： result - 用来保存计算出的结果,目前仅支持X方向的角度
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/09/05	     V1.0	  韦东山	      创建
 ***********************************************************************/
void MPU6050_ParseData(int16_t AccX, int16_t AccY, int16_t AccZ, int16_t GyroX, int16_t GyroY, int16_t GyroZ, struct mpu6050_data *result)
{
	if (result)
	{
		result->angle_x = (int32_t)(acos((double)((double)(AccX + MPU6050_X_ACCEL_OFFSET) / 16384.0)) * 57.29577);
	}
}



/**********************************************************************
 * 函数名称： mpu6050_task
 * 功能描述： MPU6050任务,它循环读取MPU6050并把数值写入队列
 * 输入参数： params - 未使用
 * 输出参数： 无
 *            无
 * 返 回 值： 0 - 成功, 其他值 - 失败
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/09/05	     V1.0	  韦东山	      创建
 ***********************************************************************/
void MPU6050_Task(void *params)
{	
    int16_t AccX;
	struct mpu6050_data result;
	int ret;
	//extern volatile int bInUsed;


    
    while (1)
    {    
	
		
		do 
		{
			/* 读数据 */
			ret = MPU6050_ReadData(&AccX, NULL, NULL, NULL, NULL, NULL);

			//bInUsed = 0;
			
			if (0 == ret)
			{
				/* 解析数据 */
				MPU6050_ParseData(AccX, 0, 0, 0, 0, 0, &result);

				/* 写队列 */
				xQueueSend(g_xQueueMPU6050, &result, 0);
				
				if (result.angle_x != 90)
					vTaskDelay(5);
			}
		} while (!ret && (result.angle_x != 90));
		
		/* delay */
		vTaskDelay(5);
	}
}

/**********************************************************************
 * 函数名称： MPU6050_Callback
 * 功能描述： MPU6050中断回调函数,它会写事件组
 * 输入参数： 无
 * 输出参数： 无
 * 返 回 值： 无
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2023/09/07	     V1.0	  韦东山	      创建
 ***********************************************************************/
//void EXTI9_5_IRQHandler()
//void MPU6050_Callback(void)
//{
//	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
//	
//	/* 设置事件组: bit0 */
//	xEventGroupSetBitsFromISR(g_xEventMPU6050, (1<<0), &xHigherPriorityTaskWoken);
//	
//	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
//}

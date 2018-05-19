/**********************************************************************************************************
                                天穹飞控 —— 致力于打造中国最好的多旋翼开源飞控
                                Github: github.com/loveuav/BlueSkyFlightControl
                                技术讨论：bbs.loveuav.com/forum-68-1.html
 * @文件     navigation_task.c
 * @说明     导航相关任务，包括姿态估计、速度估计和位置估计
 * @版本  	 V1.0
 * @作者     BlueSky
 * @网站     bbs.loveuav.com
 * @日期     2018.05 
**********************************************************************************************************/
#include "TaskConfig.h"

#include "ahrs.h"
#include "navigation.h"
#include "gyroscope.h"
#include "magnetometer.h"
#include "flightStatus.h"

xTaskHandle navigationTask;
xTaskHandle flightStatusTask;

/**********************************************************************************************************
*函 数 名: vNavigationTask
*功能说明: 导航计算相关任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(vNavigationTask, pvParameters) 
{	
	Vector3f_t* gyro;
	Vector3f_t* acc;

	vTaskDelay(500);
	
    //姿态估计参数初始化
    AHRSInit();
	//导航参数初始化
	NavigationInit();
    
	for(;;)
	{
		//从消息队列中获取数据
		xQueueReceive(messageQueue[GYRO_DATA_PRETREAT], &gyro, (3 / portTICK_RATE_MS)); 
		xQueueReceive(messageQueue[ACC_DATA_PRETREAT], &acc, (3 / portTICK_RATE_MS)); 	

		//姿态估计
		AttitudeEstimate(*gyro, *acc, MagGetData());
		
		//等待系统初始化完成
		if(GetInitStatus() == INIT_FINISH)
		{
			//飞行速度估计
			VelocityEstimate();
            //位置估计
            PositionEstimate();
		}
	}
}

/**********************************************************************************************************
*函 数 名: vFlightStatusTask
*功能说明: 飞行状态检测相关任务
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
portTASK_FUNCTION(vFlightStatusTask, pvParameters) 
{	
	portTickType xLastWakeTime;
	static uint16_t count = 0;
	
	xLastWakeTime = xTaskGetTickCount();
    
	for(;;)
	{
        //飞行器放置状态检测
        PlaceStausCheck(GyroLpfGetData());
        
        count++;
        
        //睡眠10ms
		vTaskDelayUntil(&xLastWakeTime, (10 / portTICK_RATE_MS));
	}
}

/**********************************************************************************************************
*函 数 名: NavigationTaskCreate
*功能说明: 导航相关任务创建
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
void NavigationTaskCreate(void)
{
	xTaskCreate(vNavigationTask, "navigation", NAVIGATION_TASK_STACK, NULL, NAVIGATION_TASK_PRIORITY, &navigationTask); 
	xTaskCreate(vFlightStatusTask, "flightStatus", FLIGHT_STATUS_TASK_STACK, NULL, FLIGHT_STATUS_TASK_PRIORITY, &flightStatusTask); 
}

/**********************************************************************************************************
*函 数 名: GetNavigationTaskStackUse
*功能说明: 获取任务堆栈使用情况
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
int16_t	GetNavigationTaskStackUse(void)
{
	return uxTaskGetStackHighWaterMark(navigationTask);
}

/**********************************************************************************************************
*函 数 名: GetFlightStatusTaskStackUse
*功能说明: 获取任务堆栈使用情况
*形    参: 无
*返 回 值: 无
**********************************************************************************************************/
int16_t	GetFlightStatusTaskStackUse(void)
{
	return uxTaskGetStackHighWaterMark(flightStatusTask);
}


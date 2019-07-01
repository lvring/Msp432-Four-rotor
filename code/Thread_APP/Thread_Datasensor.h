#ifndef __THREAD_Datasensor_H
#define __THREAD_Datasensor_H
#include "RTE_Include.h"
#if RTE_USE_OS == 1

extern osThreadId_t ThreadIDDataSensor;
extern const osThreadAttr_t DataSensorTask;
extern __NO_RETURN void ThreadTaskDatasensor(void *argument);

#endif
#endif

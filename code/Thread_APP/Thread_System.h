#ifndef __THREAD_SYSTEM_H
#define __THREAD_SYSTEM_H
#include "RTE_Include.h"
#if RTE_USE_OS == 1
extern osThreadId_t ThreadIDSYS;
extern const osThreadAttr_t SystemThreadControl;
extern __NO_RETURN void ThreadTaskSYS(void *argument);

extern osThreadId_t ThreadIDStabilizer;
extern const osThreadAttr_t stabilizerTask;
extern __NO_RETURN void ThreadTaskStabilizer(void *argument);

extern osThreadId_t ThreadIDDataSensor;
extern const osThreadAttr_t DataSensorTask;
extern __NO_RETURN void ThreadTaskDatasensor(void *argument);
#endif
#endif

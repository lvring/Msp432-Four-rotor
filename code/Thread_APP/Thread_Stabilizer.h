#ifndef __THREAD_stabilizer_H
#define __THREAD_stabilizer_H
#include "RTE_Include.h"
#if RTE_USE_OS == 1

extern osThreadId_t ThreadIDStabilizer;
extern const osThreadAttr_t stabilizerTask;
extern __NO_RETURN void ThreadTaskStabilizer(void *argument);


#endif
#endif

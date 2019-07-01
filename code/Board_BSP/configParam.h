#ifndef _configParam_H_
#define _configParam_H_


typedef struct 
{
	float kp;
	float ki;
	float kd;
} pidInit_t;

typedef struct
{
	pidInit_t roll;
	pidInit_t pitch;	
	pidInit_t yaw;	
} pidParam_t;

typedef struct	
{
	pidParam_t pidAngle;	/*½Ç¶ÈPID*/	
	pidParam_t pidRate;		/*½ÇËÙ¶ÈPID*/	
} configParam_t;


extern configParam_t configParam;

#endif


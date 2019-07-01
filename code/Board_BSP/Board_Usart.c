#include "Board_Usart.h"
//Baudrate set
//	  {Baudrate,inputClockFreq,prescalar,hwRegUCBRFx,hwRegUCBRSx,oversampling}	
//    {115200, 12000000,  6,  8,  32, 1},
//    {9600,   12000000, 78,  2,   0, 1},

static Board_Usart_Handle_t BSP_USARTControl[COM_N]=
{
  // Name  ,                           
  {COM_1 ,
	EUSCI_A0,
	EUSCI_A0_BASE,
	GPIO_PORT_P1, 
	GPIO_PIN3,
	GPIO_PIN2,
	EUSCI_A_UART_CLOCKSOURCE_SMCLK,
	6,8,32,                    
	EUSCI_A_UART_LSB_FIRST,
	EUSCI_A_UART_ONE_STOP_BIT,
	EUSCI_A_UART_NO_PARITY,
	EUSCI_A_UART_MODE,
	EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION}, 
  // Name  , 	
  {COM_2 ,
	EUSCI_A1 ,
	EUSCI_A1_BASE,
	GPIO_PORT_P2, 
	GPIO_PIN3 , 
	GPIO_PIN2 ,
	EUSCI_A_UART_CLOCKSOURCE_SMCLK,
	78,2,0,
	EUSCI_A_UART_LSB_FIRST,
	EUSCI_A_UART_ONE_STOP_BIT,
	EUSCI_A_UART_NO_PARITY,
	EUSCI_A_UART_MODE ,
	EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION}, 
};

static void Board_Usart_HardInit(Board_Usart_NAME_e usart_name)
{ 	
	eUSCI_UART_Config uartConfig;
	

//	if(BSP_USARTControl[usart_name].moduleInstance==EUSCI_A0_BASE)
//		CS_setDCOCenteredFrequency(BSP_USARTControl[usart_name].GpioClk);
//	else if(BSP_USARTControl[usart_name].moduleInstance==EUSCI_A1_BASE)
//		CS_setDCOCenteredFrequency(BSP_USARTControl[usart_name].GpioClk);
	
	//CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_12);
	
	GPIO_setAsPeripheralModuleFunctionInputPin(BSP_USARTControl[usart_name].selectedPort,
	BSP_USARTControl[usart_name].ComPintx|BSP_USARTControl[usart_name].ComPinrx,GPIO_PRIMARY_MODULE_FUNCTION);
	
	uartConfig.selectClockSource = BSP_USARTControl[usart_name].ClockSource;
	uartConfig.clockPrescalar = BSP_USARTControl[usart_name].BRDIV;
	uartConfig.firstModReg= BSP_USARTControl[usart_name].UCxBRF;
	uartConfig.secondModReg =BSP_USARTControl[usart_name].UCxBRS;
	uartConfig.parity = BSP_USARTControl[usart_name].Parity;
	uartConfig.msborLsbFirst = BSP_USARTControl[usart_name].WordLength;
	uartConfig.numberofStopBits = BSP_USARTControl[usart_name].StopBits;
	uartConfig.uartMode = BSP_USARTControl[usart_name].Mode;
	uartConfig.overSampling = BSP_USARTControl[usart_name].HardwareFlowControl;
	
	UART_initModule(BSP_USARTControl[usart_name].EUSCIx_Base,&uartConfig);
	
	UART_enableModule(BSP_USARTControl[usart_name].EUSCIx_Base);
	
}


static void Board_Usart_InterruptInit(Board_Usart_NAME_e usart_name)
{
	if(BSP_USARTControl[usart_name].EUSCIx_Base==EUSCI_A0_BASE)
	{
		UART_enableInterrupt(BSP_USARTControl[usart_name].EUSCIx_Base, 
															EUSCI_A_UART_RECEIVE_INTERRUPT);
		Interrupt_enableInterrupt(INT_EUSCIA0);
	}		
	if(BSP_USARTControl[usart_name].EUSCIx_Base==EUSCI_A1_BASE)
	{
				UART_enableInterrupt(BSP_USARTControl[usart_name].EUSCIx_Base, 
															EUSCI_A_UART_RECEIVE_INTERRUPT);
		Interrupt_enableInterrupt(INT_EUSCIA1);	
	}
}
static Board_Usart_NAME_e usart_timerfucid;
static void ComTimer_Callback(void* arg)
{
	Board_Usart_NAME_e* usart_name=(Board_Usart_NAME_e *)arg;
	RTE_MessageQuene_In(&BSP_USARTControl[*usart_name].\
													UsartData.ComQuene,
													BSP_USARTControl[*usart_name].\
													UsartData.pu8Databuf,
													BSP_USARTControl[*usart_name].\
													UsartData.u16Datalength);
	
	memset(BSP_USARTControl[*usart_name].UsartData.pu8Databuf,
				0,BSP_USARTControl[*usart_name].UsartData.u16Datalength);
	BSP_USARTControl[*usart_name].UsartData.u16Datalength=0;
	
}
void Board_Usart_Init(Board_Usart_NAME_e usart_name) {
	//ÉêÇëÄÚ´æ
	BSP_USARTControl[usart_name].UsartData.pu8Databuf = RTE_BGetz(MEM_RTE,32);
	RTE_AssertParam(BSP_USARTControl[usart_name].UsartData.pu8Databuf);
	BSP_USARTControl[usart_name].UsartData.u16Datalength = 0;
	RTE_MessageQuene_Init(&BSP_USARTControl[usart_name].UsartData.ComQuene,128);
	Board_Usart_HardInit(usart_name);
	Board_Usart_InterruptInit(usart_name);
	RTE_RoundRobin_CreateTimer(0,"ComTimer",100,0,0,ComTimer_Callback,(void *)&usart_timerfucid);
}
Board_Usart_Data_t *Board_Usart_ReturnQue(Board_Usart_NAME_e usart_name)
{
	return &BSP_USARTControl[usart_name].UsartData;
}
void Board_Usart_Puts(Board_Usart_NAME_e usart_name, char* str,uint16_t len) {
	/* Go through entire string */
	while (len) {
		UART_transmitData(BSP_USARTControl[usart_name].EUSCIx_Base,*str++);
		while(!(BSP_USARTControl[usart_name].EUSCIx->IFG&EUSCI_A_IFG_TXIFG));
		len--;
	}
}
void Board_Usart_Putc(Board_Usart_NAME_e usart_name, uint8_t Data)
{
	UART_transmitData(BSP_USARTControl[usart_name].EUSCIx_Base,Data);
	while(!(BSP_USARTControl[usart_name].EUSCIx->IFG&EUSCI_A_IFG_TXIFG));
}

static void Board_Usart_RecCallback(Board_Usart_NAME_e usart_name)
{
	usart_timerfucid = usart_name;
	BSP_USARTControl[usart_name].UsartData.u16Datalength++;
	BSP_USARTControl[usart_name].UsartData.pu8Databuf[BSP_USARTControl[usart_name].UsartData.u16Datalength++] = 
			UART_receiveData(BSP_USARTControl[usart_name].EUSCIx_Base);
	RTE_RoundRobin_ResetTimer(0,RTE_RoundRobin_GetTimerID(0,"ComTimer"));
}
void EUSCIA0_IRQHandler(void) {
	if(EUSCI_A0->IFG & EUSCI_A_IFG_RXIFG)
	{
		//UART_clearInterruptFlag(EUSCI_A0_BASE, UART_getEnabledInterruptStatus(EUSCI_A0_BASE));
		Board_Usart_RecCallback(COM_1);
	}
}

void EUSCIA1_IRQHandler(void) {
	if(EUSCI_A1->IFG & EUSCI_A_IFG_RXIFG)
	{
		//UART_clearInterruptFlag(EUSCI_A0_BASE, UART_getEnabledInterruptStatus(EUSCI_A0_BASE));
		Board_Usart_RecCallback(COM_2);
	}
}
	

#include "tle82452.h"


//Just read
Reg_ICVID reg_ICVID;
Reg_Diag regDiag;
Reg_FeedBack regFeedBack1,regFeedBack2;

//RW
Reg_Config regConfig;
Reg_CLK_Div reg_CLK_Div =
	{
		.regID = REG_CLK_DIVIDER};
Reg_Setpoint reg_Setpoint_load1, reg_Setpoint_load2;


uint32_t rdTemp;
uint32_t wrTemp;
uint8_t errorTemp = 0;
uint8_t tle82452_Init_Steps=1;
uint8_t faultnPin = 0;
/*****************************************************************************/
/**
 * Function Name: drv_TLE82452_ReadWriteData(uint32_t wr, uint32_t *rd)
 * Description:   spi driver
 *
 * Param:   wr: data write
 * Param:   *rd: data read
 * Return:  none
 * Author:	
 *****************************************************************************/
void drv_TLE82452_ReadWriteData(uint32_t wr, uint32_t *rd)
{
	/*Your spi driver code.*/
}
/*****************************************************************************/
/**
 * Function Name: drv_TLE82452_CLKOUT_Start(void)
 * Description:   8~40MHz. The CLK pin must be connected to a precise clock signal.  
 *                You can use the clkout function of your microcontroller.
 * 
 * Return:  none
 * Author:	
 *****************************************************************************/
void drv_TLE82452_CLKOUT_Start(void)
{
	/*Your CLKOUT driver code.*/
}





uint8_t drv_TLE82452_ReadReg(uint8_t addr, uint32_t *rd)
{
	uint32_t rd_unused;
	wrTemp = addr << 24;

	//Request
	drv_TLE82452_ReadWriteData(wrTemp, &rd_unused);
	//Get
	drv_TLE82452_ReadWriteData(0, &rdTemp);
	if ((rdTemp >> 24) == addr)
	{
		*rd = rdTemp;
		return 0;//ok
	}
	else
	{
		return 1;//error
	}
}

uint8_t drv_TLE82452_WriteReg(uint8_t addr, uint32_t wr)
{
	uint32_t rd_unused;
	wrTemp = addr << 24;
	wrTemp |=0x80000000;
	wrTemp |= wr;
	
	//Request
	drv_TLE82452_ReadWriteData(wrTemp, &rd_unused);
	//Get
	drv_TLE82452_ReadWriteData(0, &rdTemp);
	
	if (rdTemp == wrTemp)
		return 0;//ok
	else
		return 1;//error
}


/*****************************************************************************/
/**
 * Function Name: void HWA_TLE82452_Init_Handler_10ms(void)
 * Description:   Initialization process, call once every 10ms
 *
 * param:	channel:load1,load2
 * param:	currentmA:0~1500
 * param:	enable:1=enables
 * Return:  none
 * Author:	Talon
 *****************************************************************************/
void HWA_TLE82452_SetCurrent_mA(uint8_t channel, uint16_t current_mA, uint8_t enable)
{
	//	When the channel is disabled, both transistors of the half bridge are turned off. The output stage is in a high output
	//	impedance state in this condition. The channel is disabled if the EN pin is 0, or the EN bit is 0, or the set point = 0.
	uint16_t setpoint = current_mA * 2047 / 1500;

	drv_TLE82452_WriteReg(REG_SETPOINT|channel, (enable << 23)|setpoint);
}

/*****************************************************************************/
/**
 * Function Name: void HWA_TLE82452_Init_Handler_10ms(void)
 * Description:   Initialization handler, call once every 10ms.
 *
 * Return:  none
 * Author:	Talon
 *****************************************************************************/
void HWA_TLE82452_Init_Handler_10ms(void)
{
	switch(tle82452_Init_Steps)
	{
		case 1:	//Step.1  Apply the clock signal to the CLK pin, then transition the RESN pin from low to high.
			//CLK
			//The CLK pin must be connected to a precise clock signal. 
			//8~40MHz
			drv_TLE82452_CLKOUT_Start();

			//RST
			//The RESN pin is the reset input for the device. If the RESN pin is low, the device is held in an internal reset state,
			//the FAULTN pin is held low, and the SPI interface is disabled. An internal pull down current source will hold the
			//RESN pin low in case the pin is open.
			//	drv_palClearPad(TLE_RST_PORT, TLE_RST_PAD);
			//	DelayUs(100);
			drv_palSetPad(TLE_RST_PORT, TLE_RST_PAD);
			tle82452_Init_Steps=2;
			break;
		case 2://Step.2	 Wait for the TPOR timer to elapse The SPI bus will begin responding after the TPOR has elapsed.
			//TPOR /ms = 65536/(FCLK/kHz)	=65536/10000KHz=6.5536ms
			//check SPI 
			errorTemp = drv_TLE82452_ReadReg(REG_ICVID, (uint32_t *)&reg_ICVID);
			if (reg_ICVID.manufacturerID == 0xC1)
			{
				tle82452_Init_Steps=3;
			}
			break;
		case 3://Step.3	 Write to the CLK-DIVIDER register via the SPI interface. Enable the watchdog and set the system clock divider.
			//The output stages are disabled until the WDEN bit is set. To operate
			//the device without the watchdog function, the WDEN bit must be set
			//to 1 and then cleared to 0.
			
			//CLK-DIVIDER REGISTER:
			errorTemp = drv_TLE82452_WriteReg(REG_CLK_DIVIDER,CLK_DEV_WDEN|Fsys_Div);
			//At this time, an auto zero will be performed.
			if(errorTemp)
			{
				break;
			}
			errorTemp = drv_TLE82452_ReadReg(REG_CLK_DIVIDER, (uint32_t *)&reg_CLK_Div);
			tle82452_Init_Steps=4;
			break;
		case 4://Step.4  Wait for TWU wake up timer to elapse TWU has elapsed when the AZ bit returns to 1 in the SPI register.
			//TWU /ms = 65536/(FSYS/kHz) + 0.1
			//Wait Device Ready.
			if(drv_palReadPad(TLE_FAULTN_PORT, TLE_FAULTN_PAD)==1)
			{
				//Set PWM reg
				errorTemp = drv_TLE82452_WriteReg(REG_PWM_PERIOD|Load1, KI_Index|(Fsys_Hz/16/Fpwm_Hz));
				errorTemp = drv_TLE82452_WriteReg(REG_PWM_PERIOD|Load2, KI_Index|(Fsys_Hz/16/Fpwm_Hz));

				//Enable Output. Optional, your also can enable it when your need.
				//The EN pin is used to enable / disable the output stages. If the EN pin is low, all of the channels are disabled and
				//(when the fault mask bit FME = 1) the FAULTN pin is pulled low. The SPI interface remains functional. However,
				//when the EN pin is low, the EN bitsin the SET-POINT registers are cleared. The EN pin can be connected to a
				//general purpose output pin of the microcontroller or to an output of a safing circuit. However, all other SPI register
				//settings remain unchanged. After the EN pin goes high the EN bits in the set point registers remain 0 until they are
				//changed to 1. The EN bits will immediately return to 0 if the EN pin is low.
				drv_palSetPad(TLE_EN_PORT, TLE_EN_PAD);
				tle82452_Init_Steps=0;//Init end.
			}
			break;
		default:
			break;
				
	}

}


/*****************************************************************************/
/**
 * Function Name: void HWA_TLE82452_Handler_10ms(void)
 * Description:   TLE82452 main handler, call once every 10ms.
 *
 * Return:  none
 * Author:	Talon
 *****************************************************************************/
void HWA_TLE82452_Handler_10ms()
{
	HWA_TLE82452_Init_Handler_10ms();
	if(!tle82452_Init_Steps||tle82452_Init_Steps>=4)
	{
		//fault handle
		//The FAULTN pin is an open drain output. This pin is pulled low when a fault is detected by the diagnosis circuit or
		//when the device is in an internal reset state. An external resistor should be connected between this pin and the
		//VIO supply.
		faultnPin = drv_palReadPad(TLE_FAULTN_PORT, TLE_FAULTN_PAD);
		if (faultnPin == 0)
		{
			drv_TLE82452_ReadReg(REG_DIAG, (uint32_t *)&regDiag);
		}
		errorTemp = drv_TLE82452_ReadReg(REG_ICVID, (uint32_t *)&reg_ICVID);
		errorTemp = drv_TLE82452_ReadReg(REG_CLK_DIVIDER, (uint32_t *)&reg_CLK_Div);
		errorTemp = drv_TLE82452_ReadReg(REG_SETPOINT|Load1, (uint32_t *)&reg_Setpoint_load1);
		errorTemp = drv_TLE82452_ReadReg(REG_SETPOINT|Load2, (uint32_t *)&reg_Setpoint_load2);
		errorTemp = drv_TLE82452_ReadReg(REG_SETPOINT|Load1, (uint32_t *)&regFeedBack1);
		errorTemp = drv_TLE82452_ReadReg(REG_SETPOINT|Load2, (uint32_t *)&regFeedBack2);
	}
}

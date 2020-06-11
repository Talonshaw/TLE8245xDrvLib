#ifndef _TLE82452_H
#define _TLE82452_H
#include <stdint.h>

//****************************IO config****************************//
#define TLE_RST_PORT PORT_B
#define TLE_RST_PAD 15

#define TLE_EN_PORT PORT_D
#define TLE_EN_PAD 15

#define TLE_CS_PORT PORT_F
#define TLE_CS_PAD 8

#define TLE_FAULTN_PORT PORT_D
#define TLE_FAULTN_PAD 0

//****************************TLE82452 config****************************//
#define CLK_DEV_WDEN (1 << 12)
//0 = Disable Watchdog (Reset value)
//1 = Enable Watchdog

#define Fsys_Div 3
//00 - divide by 8 (Reset value)
//01 - divide by 6
//10 - divide by 4
//11 - divide by 2

/*KI gains of 1, 1/2, 1/4, 1/8, 1/16, 1/32, and 1/64 can be selected in the PWM Period Register. 
The KI value of 1,KI_index =0, has the fastest response time, the KI value of 1/64, KI_index=6, 
has the slowest response time, butwith less overshoot and less ringing. KI_index = 6 is the 
recommended setting for initial evaluation.*/
#define KI_Index 6
//KI = 2^-KI_index. Maximum value = 6.	KI_index reset value = 010B

#define Fclk_Hz 10000000 //CLK pin input clock frequency, 8~40Mhz
#define Fsys_Hz 5000000  //SYS clock frequency, 4~6MHz,Fsys=Fclk/Fsys_Div

/*The current controller regulates the load current by alternatively turning on the drive switch 
and the recirculationswitch. The on time of the drive switch is determined by the integrated PWM 
period controller. The off time of thetransistor is determined by the average current controller. 
When the average load current over the current PWMperiod is equal to the setpoint during 
freewheeling, the drive transistor is turned on again and the next PWM cycleis started.*/
#define Fpwm_Hz 1000 //LOADx PWM Frequency	100–4000Hz ，此项在初始化时必须设置，否则不报错但无输出。



//*****************************register addr******************************//
//public reg
#define REG_ICVID 0x00
#define REG_CONFIG 0x01
#define REG_DIAG 0x02
#define REG_CLK_DIVIDER 0x03
#define REG_CALIBRATION 0x05

//channel
#define Load1 0x1
#define Load2 0x2
//channel reg
#define REG_SETPOINT 0x10
#define REG_DITHER 0x18
#define REG_INTEGRATOR_LIMIT 0x20
#define REG_PWM_PERIOD 0x28
#define REG_INTEGRATOR_THRESHOLD_AND_OPEN_ON 0x30
#define REG_AUTOZERO 0x38
#define REG_FEEDBACK 0x40



//logic
#define TLE82452_ENABLE 1
#define TLE82452_DISABLE 0

typedef struct
{
    /* byte0 */
    uint8_t rw : 1;    //bit7
    uint8_t regID : 7; //bit6~0

    /* byte1 */
    uint8_t manufacturerID : 8; //bit7~0

    /* byte2 */
    uint8_t version : 8; //bit7~0

    /* byte3 */
    uint8_t : 7;     //bit7~bit1
    uint8_t WDS : 1; //bit0

} Reg_ICVID;

typedef struct
{                      /* byte0 */
    uint8_t rw : 1;    //bit7
    uint8_t regID : 7; //bit6~0

    /* byte1 */
    uint8_t : 8; //bit7~0

    /* byte2 */
    uint8_t IDIAG2 : 1; //bit7
    uint8_t IDIAG1 : 1; //bit6
    uint8_t : 1;        //bit5
    uint8_t SR2 : 2;    //bit4~3
    uint8_t SR1 : 2;    //bit2~1
    uint8_t : 1;        //bit0

    /* byte3 */
    uint8_t : 1;     //bit7
    uint8_t FME : 1; //bit6
    uint8_t FM2 : 1; //bit5
    uint8_t FM1 : 1; //bit4
    uint8_t : 1;     //bit3
    uint8_t HL2 : 1; //bit2
    uint8_t HL1 : 1; //bit1
    uint8_t : 1;     //bit0

} Reg_Config;

typedef struct
{                      /* byte0 */
    uint8_t rw : 1;    //bit7
    uint8_t regID : 7; //bit6~0

    /* byte1 */
    uint8_t CRC0 : 1; //bit7
    uint8_t RST0 : 1; //bit6
    uint8_t CPUV : 1; //bit5
    uint8_t CPW : 1;  //bit4
    uint8_t OVB : 1;  //bit3
    uint8_t : 1;      //bit2
    uint8_t RE2 : 1;  //bit1
    uint8_t RE1 : 1;  //bit0

    /* byte2 */
    uint8_t : 1;        //bit7
    uint8_t UV2 : 1;    //bit6
    uint8_t UV1 : 1;    //bit5
    uint8_t : 1;        //bit4
    uint8_t OT2 : 1;    //bit3
    uint8_t OT1 : 1;    //bit2
    uint8_t : 1;        //bit1
    uint8_t OLOFF2 : 1; //bit0

    /* byte3 */
    uint8_t OLOFF1 : 1; //bit7
    uint8_t : 1;        //bit6
    uint8_t OLSB2 : 1;  //bit5
    uint8_t OLSB1 : 1;  //bit4
    uint8_t : 1;        //bit3
    uint8_t OVC2 : 1;   //bit2
    uint8_t OVC1 : 1;   //bit1
    uint8_t : 1;        //bit0

} Reg_Diag;

typedef struct
{
    /* byte3 */
    uint8_t rw : 1;    //bit7
    uint8_t regID : 7; //bit6~0

    /* byte2 */
    uint8_t : 8; //bit7~0

    /* byte1 */
    uint8_t : 3;
    uint8_t WDEN : 1;
    uint8_t MH : 4;

    /* byte0 */
    uint8_t ML : 2;
    uint8_t N : 4;
    uint8_t Fsys_div : 2;

} Reg_CLK_Div;

typedef struct
{                      /* byte0 */
    uint8_t rw : 1;    //bit7
    uint8_t regID : 7; //bit6~0

    /* byte1 */
    uint8_t EN : 1;
    uint8_t AL : 1;
    uint8_t : 6;

    /* byte2 */
    uint8_t : 5;
    uint8_t setpoint_h : 3;

    /* byte3 */
    uint8_t setpoint_l : 8;

} Reg_Setpoint;

typedef struct
{                      /* byte0 */
    uint8_t rw : 1;    //bit7
    uint8_t regID : 7; //bit6~0

    /* byte1 */
    uint8_t data1 : 8;

    /* byte2 */
    uint8_t data2 : 8;

    /* byte3 */
    uint8_t data3 : 8;

} Reg_FeedBack;


void HWA_TLE82452_Handler_10ms(void);
void HWA_TLE82452_SetCurrent_mA(uint8_t channel, uint16_t current_mA, uint8_t enable);

#endif

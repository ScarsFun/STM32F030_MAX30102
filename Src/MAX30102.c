#include"MAX30102.h"

void max30102_WriteReg(uint8_t uch_addr, uint8_t uch_data)
{
	I2C1_WriteBytesToAddr ( MAX30102_I2C_ADDR, uch_addr, &uch_data, 1);
}

void max30102_ReadReg(uint8_t uch_addr, uint8_t *puch_data)
{
	I2C1_ReadBytesFromAddr(MAX30102_I2C_ADDR, uch_addr, puch_data, 1);
}

void max30102_ReadFifo(uint32_t* pun_red_led, uint32_t* pun_ir_led)
{
    uint32_t un_temp;
    uint8_t uch_temp;
    uint8_t ach_i2c_data[6];
    *pun_ir_led = 0;
    *pun_red_led = 0;

    max30102_ReadReg(REG_INTR_STATUS_1, &uch_temp);
    max30102_ReadReg(REG_INTR_STATUS_2, &uch_temp);

    //IIC_ReadBytes(I2C_WRITE_ADDR,REG_FIFO_DATA,(u8 *)ach_i2c_data,6);
    I2C1_ReadBytesFromAddr(MAX30102_I2C_ADDR, REG_FIFO_DATA, ach_i2c_data, 6);

    un_temp = ach_i2c_data[0];
    un_temp <<= 16;
    *pun_red_led += un_temp;
    un_temp = ach_i2c_data[1];
    un_temp <<= 8;
    *pun_red_led += un_temp;
    un_temp = ach_i2c_data[2];
    *pun_red_led += un_temp;

    un_temp = ach_i2c_data[3];
    un_temp <<= 16;
    *pun_ir_led += un_temp;
    un_temp = ach_i2c_data[4];
    un_temp <<= 8;
    *pun_ir_led += un_temp;
    un_temp = ach_i2c_data[5];
    *pun_ir_led += un_temp;
    *pun_red_led &= 0x03FFFF; //Mask MSB [23:18]
    *pun_ir_led &= 0x03FFFF; //Mask MSB [23:18]
}

void max30102_Init(void)
{
	max30102_WriteReg(REG_MODE_CONFIG,0x40); //reset
    LL_mDelay(20);
	max30102_WriteReg(REG_INTR_ENABLE_1,0xc0);	// INTR setting
	max30102_WriteReg(REG_INTR_ENABLE_2,0x00);
	max30102_WriteReg(REG_FIFO_WR_PTR,0x00);  	//FIFO_WR_PTR[4:0]
	max30102_WriteReg(REG_OVF_COUNTER,0x00);  	//OVF_COUNTER[4:0]
	max30102_WriteReg(REG_FIFO_RD_PTR,0x00);  	//FIFO_RD_PTR[4:0]
	max30102_WriteReg(REG_FIFO_CONFIG,0x4f);  	//sample avg = 4, fifo rollover=false, fifo almost full = 17
	max30102_WriteReg(REG_MODE_CONFIG,0x03);  	//0x02 for Red only, 0x03 for SpO2 mode 0x07 multimode LED
	max30102_WriteReg(REG_SPO2_CONFIG,0x27);  	// SPO2_ADC range = 4096nA, SPO2 sample rate (100 Hz), LED pulseWidth (400uS)  
	max30102_WriteReg(REG_LED1_PA,0x24);   	//Choose value for ~ 7mA for LED1
	max30102_WriteReg(REG_LED2_PA,0x24);   	// Choose value for ~ 7mA for LED2
	max30102_WriteReg(REG_PILOT_PA,0x7f);   	// Choose value for ~ 25mA for Pilot LED
}

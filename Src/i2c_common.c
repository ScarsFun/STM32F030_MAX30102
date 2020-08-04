# include "i2c_common.h"


#define TIME_OUT_INIT 100U
#define TIME_OUT_MANAGE() {TimeOutChecker(pTimeOut, I2C_ERR);} timeOut = TIME_OUT_INIT;

static uint8_t timeOut = TIME_OUT_INIT;
static uint8_t* pTimeOut = &timeOut, flag;

void I2C1_WriteBytesToAddr (uint8_t iic_addr, uint8_t addr,uint8_t *buf, uint16_t len)
{
  while (LL_I2C_IsActiveFlag_BUSY(I2C1) != RESET) TIME_OUT_MANAGE();
  LL_I2C_HandleTransfer(I2C1, (iic_addr) | I2C_REQUEST_WRITE, LL_I2C_ADDRSLAVE_7BIT, 
  						1, LL_I2C_MODE_RELOAD, LL_I2C_GENERATE_START_WRITE);
  
  while(!(LL_I2C_IsActiveFlag_TXE(I2C1))) TIME_OUT_MANAGE();
  LL_I2C_TransmitData8(I2C1, (uint8_t) addr);

  while(!(LL_I2C_IsActiveFlag_TCR(I2C1))) TIME_OUT_MANAGE();
  
  LL_I2C_HandleTransfer(I2C1, iic_addr, LL_I2C_ADDRSLAVE_7BIT, 
  						len, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_NOSTARTSTOP);
 for (uint16_t i = 0; i < len; i++) 
  {
  	while(!(LL_I2C_IsActiveFlag_TXE(I2C1))) TIME_OUT_MANAGE();
    LL_I2C_TransmitData8(I2C1, buf[i]);
  }
  while(!(LL_I2C_IsActiveFlag_STOP(I2C1))) TIME_OUT_MANAGE();
  LL_I2C_ClearFlag_STOP(I2C1);
  }

void I2C1_ReadBytesFromAddr(uint8_t iic_addr, uint16_t addr, uint8_t *buf, uint8_t len) {

    LL_I2C_ClearFlag_STOP(I2C1);

    LL_I2C_HandleTransfer(I2C1, (iic_addr) | I2C_REQUEST_READ, LL_I2C_ADDRSLAVE_7BIT, 
    						1, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_WRITE);
     while(LL_I2C_IsActiveFlag_ADDR(I2C1)) TIME_OUT_MANAGE();
  	LL_I2C_TransmitData8(I2C1, (uint8_t) addr);

   while(!(LL_I2C_IsActiveFlag_TXE(I2C1))) TIME_OUT_MANAGE();
			LL_I2C_ClearFlag_STOP(I2C1);

    LL_I2C_HandleTransfer(I2C1, iic_addr, LL_I2C_ADDRSLAVE_7BIT, 
    					  len, LL_I2C_MODE_AUTOEND, LL_I2C_GENERATE_START_READ);

    do {
        while( !(LL_I2C_IsActiveFlag_RXNE(I2C1))) TIME_OUT_MANAGE();
        *buf = LL_I2C_ReceiveData8(I2C1);
        buf++;
    } while(--len > 0);
   // while(!(LL_I2C_IsActiveFlag_STOP(I2C1))){};
    LL_I2C_ClearFlag_STOP(I2C1);
}

void TimeOutChecker(uint8_t* timeout_checker, uint8_t err_code)
{
	/* Check Systick counter flag to decrement the time-out value */
	if (LL_SYSTICK_IsActiveCounterFlag())
		{
			if((*timeout_checker)-- == I2C_ERR)
			{
			  I2C_Error_Handler(err_code);
			}
		}
}


/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void I2C_Error_Handler(uint8_t code)
{
	// USER CODE BEGIN I2C_Error_Handler_Debug
  // User can add his own implementation to report the  error return state
	__asm("nop");
  // USER CODE END Error_Handler_Debug
}

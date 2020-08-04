/* USER CODE BEGIN Header */
/**

 Simple Oxymeter STM32F030F4 by ScarsFun V0.99 4 august 2020
 PA9  scl --> SSD1306 --> MAX30102
 PA10 sda --> SSD1306 --> MAX30102
 PA4  --> INT pin MAX30102
 PA5  --> LED data monitor

/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "algorithm_by_RF.h"
#include "MAX30102.h"
#include "ssd1306.h"
#include "fontsA.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
//uint32_t elapsedTime,timeStart;

uint32_t aun_ir_buffer[100]; //infrared LED sensor data
uint32_t aun_red_buffer[100]; //red LED sensor data
//float old_n_spo2;  // Previous SPO2 value
uint8_t uch_dummy;
float n_spo2, ratio, correl; //SPO2 value

int8_t ch_spo2_valid; //indicator to show if the SPO2 calculation is valid
int32_t n_heart_rate; //heart rate value
int8_t ch_hr_valid; //indicator to show if the heart rate calculation is valid
int32_t i;
char hr_str[20];
const uint8_t heart[8] = { 0x1E, 0x3F, 0x7F, 0xFE, 0xFE, 0x7F, 0x3F, 0x1E };
const uint8_t blank[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
    /* USER CODE BEGIN 1 */

    /* USER CODE END 1 */

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    /* System interrupt init*/

    /* USER CODE BEGIN Init */

    /* USER CODE END Init */

    /* Configure the system clock */
    SystemClock_Config();

    /* USER CODE BEGIN SysInit */

    /* USER CODE END SysInit */

    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_I2C1_Init();
    /* USER CODE BEGIN 2 */
    max30102_Init();
    ssd1306_Init();
    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    //ssd1306_RestoreFullWindow();
    ssd1306_Fill(Black);
    ssd1306_DrawText("  STM32", FONT_MED, 5, 0, White);
    ssd1306_UpdateScreen();
    ssd1306_DrawText(" OXIMETER", FONT_MED, 5, 16, White);
    ssd1306_UpdateScreen(); 
		while (1)
    {
        /* USER CODE END WHILE */

        /* USER CODE BEGIN 3 */
        //read the first 100 samples, and determine the signal range
        for (i = 0; i < BUFFER_SIZE; i++) {
            while (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_4)) {
            };

            // my max30102 module has ir, red inverted. modify max30102_ReadFifo((aun_red_buffer+i), (aun_ir_buffer+i));
            max30102_ReadFifo((aun_ir_buffer + i), (aun_red_buffer + i)); //read from MAX30102 FIFO
        }
        //calculate heart rate and SpO2 after first 100 samples (first 4 seconds of samples)
        rf_heart_rate_and_oxygen_saturation(aun_ir_buffer, BUFFER_SIZE, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid, &ratio, &correl);

        ssd1306_Fill(Black);
        ssd1306_DrawText("data...", FONT_MED, 0, 5, White);
        ssd1306_UpdateScreen();
        //Continuously taking samples from MAX30102.  Heart rate and SpO2 are calculated every 1 second
        while (1) {
            //dumping the first 25 sets of samples in the memory and shift the last 75 sets of samples to the top
            for (i = 25; i < 100; i++) {
                aun_red_buffer[i - 25] = aun_red_buffer[i];
                aun_ir_buffer[i - 25] = aun_ir_buffer[i];
            }
            //take 25 sets of samples before calculating the heart rate.
            LL_GPIO_SetOutputPin(GPIOA, LL_GPIO_PIN_5);
            for (i = 75; i < 100; i++) {

                while (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_4)) {
                };

                max30102_ReadFifo((aun_ir_buffer + i), (aun_red_buffer + i)); //read from MAX30102 FIFO
            }
            LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);
            rf_heart_rate_and_oxygen_saturation(aun_ir_buffer, BUFFER_SIZE, aun_red_buffer, &n_spo2, &ch_spo2_valid, &n_heart_rate, &ch_hr_valid, &ratio, &correl);
            if (ch_hr_valid && ch_spo2_valid) {

                //ssd1306_DrawDirect(45, 2, heart);
                //ssd1306_RestoreFullWindow();
                //sprintf(hr_str,"%3d Hr %.1f Sp",n_heart_rate,n_spo2);
                ssd1306_Fill(Black);
                ssd1306_DrawChar(45, FONT_BIG, 55, 0, White);
                sprintf(hr_str, "%d", n_heart_rate);
                ssd1306_DrawText(hr_str, FONT_BIG, 10, 0, White);
                ssd1306_DrawText("56", FONT_MICRO, 10, 24, White);
                sprintf(hr_str, "%.1f", n_spo2);
                ssd1306_DrawText(hr_str, FONT_BIG, 78, 0, White);
                ssd1306_DrawText("01234", FONT_MICRO, 78, 24, White);
                //sprintf(hr_str,"%3d",n_heart_rate);
                ssd1306_UpdateScreen();
                __ASM("nop");
            }
            else {

                //ssd1306_DrawDirect(45, 2, blank);
                //ssd1306_RestoreFullWindow();
                ssd1306_DrawChar(44, FONT_BIG, 55, 0, White);
                ssd1306_UpdateScreen();
            }
        }
    }
    /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    if (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1) {
        Error_Handler();
    }
    LL_RCC_HSI_Enable();

    /* Wait till HSI is ready */
    while (LL_RCC_HSI_IsReady() != 1) {
    }
    LL_RCC_HSI_SetCalibTrimming(16);
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2, LL_RCC_PLL_MUL_12);
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1) {
    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    }
    LL_Init1msTick(48000000);
    LL_SetSystemCoreClock(48000000);
    LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_HSI);
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

    /* USER CODE BEGIN I2C1_Init 0 */

    /* USER CODE END I2C1_Init 0 */

    LL_I2C_InitTypeDef I2C_InitStruct = { 0 };

    LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);
    /**I2C1 GPIO Configuration  
  PA9   ------> I2C1_SCL
  PA10   ------> I2C1_SDA 
  */
    GPIO_InitStruct.Pin = LL_GPIO_PIN_9;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = LL_GPIO_PIN_10;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_ALTERNATE;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_OPENDRAIN;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_UP;
    GPIO_InitStruct.Alternate = LL_GPIO_AF_4;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* Peripheral clock enable */
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);

    /* USER CODE BEGIN I2C1_Init 1 */

    /* USER CODE END I2C1_Init 1 */
    /** I2C Initialization 
  */
    LL_I2C_DisableOwnAddress2(I2C1);
    LL_I2C_DisableGeneralCall(I2C1);
    LL_I2C_EnableClockStretching(I2C1);
    I2C_InitStruct.PeripheralMode = LL_I2C_MODE_I2C;
    I2C_InitStruct.Timing = 0x0000020B;
    I2C_InitStruct.AnalogFilter = LL_I2C_ANALOGFILTER_ENABLE;
    I2C_InitStruct.DigitalFilter = 0;
    I2C_InitStruct.OwnAddress1 = 0;
    I2C_InitStruct.TypeAcknowledge = LL_I2C_ACK;
    I2C_InitStruct.OwnAddrSize = LL_I2C_OWNADDRESS1_7BIT;
    LL_I2C_Init(I2C1, &I2C_InitStruct);
    LL_I2C_EnableAutoEndMode(I2C1);
    LL_I2C_SetOwnAddress2(I2C1, 0, LL_I2C_OWNADDRESS2_NOMASK);
    /* USER CODE BEGIN I2C1_Init 2 */
    LL_I2C_Enable(I2C1);
    /* USER CODE END I2C1_Init 2 */
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = { 0 };

    /* GPIO Ports Clock Enable */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_4;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    LL_GPIO_ResetOutputPin(GPIOA, LL_GPIO_PIN_5);

    /**/
    GPIO_InitStruct.Pin = LL_GPIO_PIN_5;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    GPIO_InitStruct.Pull = LL_GPIO_PULL_NO;
    LL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
    /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */

    /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
    /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f0xx_hal.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include "peripheral.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define SPI_BLOCK_SIZE        10
/* SPI block words */
#define SPI_COMMAND_WORD      0
#define SPI_DAC1_CH_A         1
#define SPI_DAC1_CH_B         2
#define SPI_DAC2_CH_A         3
#define SPI_DAC2_CH_B         4
#define SPI_DAC3              5
#define SPI_POT               6

/* SPI command word flags */
#define SPI_COMMAND_OUTPUT    0x01
#define SPI_COMMAND_DAC1_CH_A 0x02
#define SPI_COMMAND_DAC1_CH_B 0x04
#define SPI_COMMAND_DAC2_CH_A 0x08
#define SPI_COMMAND_DAC2_CH_B 0x10
#define SPI_COMMAND_DAC3      0x20
#define SPI_COMMAND_POT       0x40

uint16_t adcData[SPI_BLOCK_SIZE];

uint16_t spiCtrl[SPI_BLOCK_SIZE];

extern ADC_HandleTypeDef hadc;
extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim6;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void Error_Handler(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

void transferComplete(void) {
	/* handle received commands */
	if (spiCtrl[SPI_COMMAND_WORD] & SPI_COMMAND_OUTPUT) {
		/* enable output relay */
		HAL_GPIO_WritePin(OUTPUT_ENABLE_GPIO_Port, OUTPUT_ENABLE_Pin,
				GPIO_PIN_SET);
	} else {
		/* disable output relay */
		HAL_GPIO_WritePin(OUTPUT_ENABLE_GPIO_Port, OUTPUT_ENABLE_Pin,
				GPIO_PIN_RESET);
	}

	/* Relay DAC data */
	if (spiCtrl[SPI_COMMAND_WORD] & SPI_COMMAND_DAC1_CH_A) {
		DAC8552_SetChannel(0, 0, spiCtrl[SPI_DAC1_CH_A]);
	}
	if (spiCtrl[SPI_COMMAND_WORD] & SPI_COMMAND_DAC1_CH_B) {
		DAC8552_SetChannel(0, 1, spiCtrl[SPI_DAC1_CH_B]);
	}
	if (spiCtrl[SPI_COMMAND_WORD] & SPI_COMMAND_DAC2_CH_A) {
		DAC8552_SetChannel(1, 0, spiCtrl[SPI_DAC2_CH_A]);
	}
	if (spiCtrl[SPI_COMMAND_WORD] & SPI_COMMAND_DAC2_CH_B) {
		DAC8552_SetChannel(1, 1, spiCtrl[SPI_DAC2_CH_B]);
	}
	if (spiCtrl[SPI_COMMAND_WORD] & SPI_COMMAND_DAC3) {
		MCP47X6_SetChannel(spiCtrl[SPI_DAC3]);
	}
	/* Relay digital potentiometer data */
	if (spiCtrl[SPI_COMMAND_WORD] & SPI_COMMAND_POT) {
		MCP41HVX1_SetWiper(spiCtrl[SPI_POT]);
	}
}

void startSPITransfer(void) {
	/* synchronize with master, this is necessary because NSS line is not available */
	HAL_SPI_Abort(&hspi2);
	/* wait for idle clock, SCL will never be low for more than 2us if transmission is active */
	uint16_t idleSince = TIM6->CNT;
	do {
		if (GPIOB->IDR & GPIO_PIN_13) {
			/* clock is high, reset timeout */
			idleSince = TIM6->CNT;
		}
	} while (TIM6->CNT - idleSince < 8);
	/* now the SPI is between transmissions and we start at the beginning of a frame */
	HAL_SPI_TransmitReceive_DMA(&hspi2, (uint8_t*) adcData, (uint8_t*) spiCtrl,
			SPI_BLOCK_SIZE);
}
/* USER CODE END PFP */

/* USER CODE BEGIN 0 */

void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
	/* apply transferred control commands */
	transferComplete();
	/* restart SPI for the next transfer */
	startSPITransfer();
}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC_Init();
  MX_I2C2_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_TIM6_Init();

  /* USER CODE BEGIN 2 */
  HAL_ADCEx_Calibration_Start(&hadc);

  HAL_TIM_Base_Start(&htim6);
  /* Initialize external DACs */
  Peripheral_Init();
  /* Start ADC sampling */
  HAL_ADC_Start_DMA(&hadc, (uint32_t*)adcData, SPI_BLOCK_SIZE);

  startSPITransfer();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  HAL_SuspendTick();
  while (1) {
	  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSI14;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSICalibrationValue = 16;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

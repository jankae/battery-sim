#include <peripheral.h>

extern SPI_HandleTypeDef hspi1;
extern I2C_HandleTypeDef hi2c2;

void Peripheral_Init(void) {
  /* set all CS pins high */
  HAL_GPIO_WritePin(CS_DAC1_GPIO_Port, CS_DAC1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(CS_DAC2_GPIO_Port, CS_DAC2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(CS_POT_GPIO_Port, CS_POT_Pin, GPIO_PIN_SET);

  /* initialize I2C DAC */
  MCP47X6_Init();
}

void DAC8552_SetChannel(uint8_t dac, uint8_t channel, uint16_t value) {
  /* SPI can operate at highest speed and has to use mode 2*/
  MODIFY_REG(SPI1->CR1, SPI_CR1_BR_Msk|SPI_CR1_CPOL_Msk|SPI_CR1_CPHA_Msk,
      SPI_BAUDRATEPRESCALER_4|SPI_POLARITY_HIGH|SPI_PHASE_1EDGE);
  /* prepare SPI transfer data */
  uint8_t data[3];
  if (!channel) {
    /* load channel A */
    data[0] = 0x10;
  } else {
    /* load channel B */
    data[0] = 0x24;
  }
  /* copy value to SPI data */
  data[1] = value >> 8;
  data[2] = value & 0xff;

  /* select DAC */
  if (!dac) {
    HAL_GPIO_WritePin(CS_DAC1_GPIO_Port, CS_DAC1_Pin, GPIO_PIN_RESET);
  } else {
    HAL_GPIO_WritePin(CS_DAC2_GPIO_Port, CS_DAC2_Pin, GPIO_PIN_RESET);
  }

  /* Transfer SPI data */
  HAL_SPI_Transmit(&hspi1, data, 3, 10);

  /* deselect DAC */
  if (!dac) {
    HAL_GPIO_WritePin(CS_DAC1_GPIO_Port, CS_DAC1_Pin, GPIO_PIN_SET);
  } else {
    HAL_GPIO_WritePin(CS_DAC2_GPIO_Port, CS_DAC2_Pin, GPIO_PIN_SET);
  }
}

void MCP41HVX1_SetWiper(uint8_t wiper) {
  /* SPI max speed is 10MHz and has to use mode 3*/
  MODIFY_REG(SPI1->CR1, SPI_CR1_BR_Msk|SPI_CR1_CPOL_Msk|SPI_CR1_CPHA_Msk,
      SPI_BAUDRATEPRESCALER_8|SPI_POLARITY_HIGH|SPI_PHASE_2EDGE);

  /* prepare SPI data */
  uint8_t data[2];
  /* write wiper register */
  data[0] = 0x00;
  data[1] = wiper;

  /* select chip */
  HAL_GPIO_WritePin(CS_POT_GPIO_Port, CS_POT_Pin, GPIO_PIN_RESET);

  /* Transfer SPI data */
  HAL_SPI_Transmit(&hspi1, data, 2, 10);

  /* deselect chip */
  HAL_GPIO_WritePin(CS_POT_GPIO_Port, CS_POT_Pin, GPIO_PIN_SET);
}

void MCP47X6_SetChannel(uint16_t value) {

  /* prepare I2C data */
  uint8_t data[2];
#if defined(MCP4706)
  data[0] = 0;
  data[1] = value & 0xff;
#elif defined(MCP4716)
  data[0] = (value>>6) & 0x0f;
  data[1] = (value<<2) & 0xff;
#elif defined(MCP4726)
  data[0] = (value >> 8) & 0x0f;
  data[1] = value & 0xff;
#endif

  /* Transfer data */
  HAL_I2C_Master_Transmit(&hi2c2, MCP47X6_ADDRESS, data, 2, 50);
}

void MCP47X6_Init(void) {
  /* prepare I2C data */
  uint8_t data[1];
  data[0] = 0x80 /* write volatile configuration bits */
  | 0x18 /* Vref, buffered */
  | 0x00 /* normal operation */
  | 0x00; /* Gain = 1 */

  /* Transfer data */
  HAL_I2C_Master_Transmit(&hi2c2, MCP47X6_ADDRESS, data, 1, 50);
}

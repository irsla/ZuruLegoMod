#include "main.h"
#include "nhd_1_8_160128B.h"

extern SPI_HandleTypeDef hspi1;
uint8_t buf[128*160*2] = {0};

int nhd_160128b_command(uint8_t c) {
  uint32_t res;
  
  // put DC LOW
  HAL_GPIO_WritePin(SCREEN_DC_GPIO_Port, SCREEN_DC_Pin, GPIO_PIN_RESET);
  res = HAL_SPI_Transmit(&hspi1, &c, 1, 100);
  if (res != 0) {
    while(1);
    return -1;
  }
  return 0;
}

int nhd_160128b_data(uint8_t c) {
  uint32_t res;

  // put DC HIGH
  HAL_GPIO_WritePin(SCREEN_DC_GPIO_Port, SCREEN_DC_Pin, GPIO_PIN_SET);
  res = HAL_SPI_Transmit(&hspi1, &c, 1, 100);
  if (res != 0) {
    while(1);
    return -1;
  }
  return 0;
}

// write to RAM command
void nhd_160128b_WriteMemoryStart(void) {
  nhd_160128b_command(0x5c);
}

// set x,y address
void nhd_160128b_SetPosition(uint8_t x_pos, uint8_t y_pos) {
  nhd_160128b_command(0x20);
  nhd_160128b_data(x_pos);
  nhd_160128b_command(0x21);
  nhd_160128b_data(y_pos);
}

// set row address start + end
void nhd_160128b_SetRowAddress(uint8_t y_start, uint8_t y_end) {
  nhd_160128b_command(0x75);
  nhd_160128b_data(y_start);
  nhd_160128b_data(y_end);
}

// set column address start + end
void nhd_160128b_SetColumnAddress(uint8_t x_start, uint8_t x_end) {
  nhd_160128b_command(0x15);
  nhd_160128b_data(x_start);
  nhd_160128b_data(x_end);
}

//fill screen with a given color. Use RGB565 format. (Useful resource: https://github.com/newdigate/rgb565_colors)
void nhd_160128b_FillScreen(uint16_t a) {
  uint8_t i;
  uint8_t j;
  uint8_t x1;
  uint8_t x2;
  
  nhd_160128b_SetColumnAddress(0x00, 0x9F);  //Columns 0-159
  nhd_160128b_SetRowAddress(0x00, 0x7f);     //Rows 0-127
  nhd_160128b_SetPosition(0, 0);
  nhd_160128b_WriteMemoryStart();
  x1 = (a>>8);
  x2 = (a & 0x00FF);
  HAL_GPIO_WritePin(SCREEN_DC_GPIO_Port, SCREEN_DC_Pin, GPIO_PIN_SET);
  for (i = 0; i < 128; i++) {
    for (j = 0; j < 160; j++) {
      HAL_SPI_Transmit(&hspi1, &x1, 1, 100);
      HAL_SPI_Transmit(&hspi1, &x2, 1, 100);
    }
  }
}

//fill screen with a given color. Use RGB565 format. (Useful resource: https://github.com/newdigate/rgb565_colors)
void nhd_160128b_FillScreen2(uint16_t a) {
  uint8_t i;
  uint8_t j;
  uint8_t x1;
  uint8_t x2;
  uint8_t res;

  
  nhd_160128b_SetColumnAddress(0x00, 0x9F);  //Columns 0-159
  nhd_160128b_SetRowAddress(0x00, 0x7f);     //Rows 0-127
  nhd_160128b_SetPosition(0, 0);
  nhd_160128b_WriteMemoryStart();
  x1 = (a>>8);
  x2 = (a & 0x00FF);
  for (i = 0; i < 128; i++) {
    for (j = 0; j < 160; j++) {
      buf[i*320 + 2 * j] = x1;
      buf[i*320 + 2 * j + 1] = x2;
      // HAL_SPI_Transmit(&hspi1, &x1, 1, 100);
      // HAL_SPI_Transmit(&hspi1, &x2, 1, 100);
    }
  }
  HAL_GPIO_WritePin(SCREEN_DC_GPIO_Port, SCREEN_DC_Pin, GPIO_PIN_SET);
  res = HAL_SPI_Transmit(&hspi1, buf, 128*160*2, 100);
  if (res != 0) {
    while(1);
  }
}

//fill screen with a given color. Use RGB565 format. (Useful resource: https://github.com/newdigate/rgb565_colors)
void nhd_160128b_DisplayImage(uint16_t *img, uint32_t img_w, uint32_t img_h) {
  uint8_t res;
  
  nhd_160128b_SetColumnAddress(0x00, 0x9F);  //Columns 0-159
  nhd_160128b_SetRowAddress(0x00, 0x7f);     //Rows 0-127
  nhd_160128b_SetPosition(0, 0);
  nhd_160128b_WriteMemoryStart();

  HAL_GPIO_WritePin(SCREEN_DC_GPIO_Port, SCREEN_DC_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_PWM_GPIO_Port, LED_PWM_Pin, GPIO_PIN_SET);
  res = HAL_SPI_Transmit(&hspi1, img, 160*128*2, 100);
  if (res != 0) {
    while(1);
  }
  HAL_GPIO_WritePin(LED_PWM_GPIO_Port, LED_PWM_Pin, GPIO_PIN_RESET);
}


void nhd_160128b_init(void) {
  // enable 17V

  HAL_GPIO_WritePin(OLED_EN_GPIO_Port, OLED_EN_Pin, GPIO_PIN_SET);  

  // put CS HIGH
  HAL_GPIO_WritePin(SCREEN_CS_GPIO_Port, SCREEN_CS_Pin, GPIO_PIN_SET);  

  HAL_GPIO_WritePin(SCREEN_DC_GPIO_Port, SCREEN_DC_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(SCREEN_RES_GPIO_Port, SCREEN_RES_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
  // put CS LOW - active LOW
  HAL_GPIO_WritePin(SCREEN_CS_GPIO_Port, SCREEN_CS_Pin, GPIO_PIN_RESET);  
  HAL_Delay(200);

  // put RES LOW
  HAL_GPIO_WritePin(SCREEN_RES_GPIO_Port, SCREEN_RES_Pin, GPIO_PIN_RESET);
  HAL_Delay(100);
  // put RES HIGH
  HAL_GPIO_WritePin(SCREEN_RES_GPIO_Port, SCREEN_RES_Pin, GPIO_PIN_SET);
  HAL_Delay(200);
  
  nhd_160128b_command(0xAE); // Set Display OFF
  nhd_160128b_command(0xA8);  //Set MUX ratio
  nhd_160128b_data(0x7F);     
  nhd_160128b_command(0xA2);  //Set Display offset
  nhd_160128b_data(0x00);
  nhd_160128b_command(0xA1);  //Set display start line
  nhd_160128b_data(0x00);
  nhd_160128b_command(0xA4);  //Normal display  
  nhd_160128b_command(0xA0);  //Set Re-map, color depth
  nhd_160128b_data(0x64);     
  nhd_160128b_command(0x81);  //Set Contrast for color"A" segment
  nhd_160128b_data(0x75);     
  nhd_160128b_command(0x82);  //Set Contrast for color"B" segment
  nhd_160128b_data(0x60);     
  nhd_160128b_command(0x83);  //Set Contrast for color"C" segment
  nhd_160128b_data(0x6A);     
  nhd_160128b_command(0x87);  //Master Contrast Current Control
  nhd_160128b_data(0x0F);     
  nhd_160128b_command(0xB9);  //use linear grayscale table
  nhd_160128b_command(0xB1);  //Set Phase1 and phase2 period adjustment
  nhd_160128b_data(0x22);
  nhd_160128b_command(0xB3);  //Set Display Clock Divide Ratio (internal clock selection)  
  nhd_160128b_data(0x40);  
  nhd_160128b_command(0xBB);  //Set Pre-charge Voltage
  nhd_160128b_data(0x08); 
  nhd_160128b_command(0xBE);  //Set VCOMH
  nhd_160128b_data(0x2F); 
  nhd_160128b_command(0xAF); //Set Display ON in mormal mode
}

void nhd_160128b_test(void) {
  /* nhd_160128b_FillScreen(GREEN); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen(WHITE); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen(RED); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen(ORANGE); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen(YELLOW); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen(BLUE); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen(VIOLET); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen(BLACK); */

  /* nhd_160128b_FillScreen2(GREEN); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen2(WHITE); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen2(RED); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen2(ORANGE); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen2(YELLOW); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen2(BLUE); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen2(VIOLET); */
  /* HAL_Delay(500); */
  /* nhd_160128b_FillScreen2(BLACK); */
}

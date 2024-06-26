/*
 * ili9341.h
 *
 *  Created on: 29 mars 2023
 *      Author: PC-MAGH
 */

#ifndef ILI9341_H_
#define ILI9341_H_

#include <stdbool.h>
#include "main.h"
#include <math.h>
#include <time.h>
#include <stdlib.h>

#define ILI9341_MADCTL_MY  0x80
#define ILI9341_MADCTL_MX  0x40
#define ILI9341_MADCTL_MV  0x20
#define ILI9341_MADCTL_ML  0x10
#define ILI9341_MADCTL_RGB 0x00
#define ILI9341_MADCTL_BGR 0x08
#define ILI9341_MADCTL_MH  0x04

/*** Redefine if necessary ***/
#define ILI9341_SPI_PORT_1 hspi1
extern SPI_HandleTypeDef ILI9341_SPI_PORT_1;

#define ILI9341_SPI_PORT_2 hspi2
extern SPI_HandleTypeDef ILI9341_SPI_PORT_2;

#define GYRO_SPI_PORT hspi3
extern SPI_HandleTypeDef GYRO_SPI_PORT;

#define ILI9341_RES_Pin_1       GPIO_PIN_1
#define ILI9341_RES_GPIO_Port_1 GPIOA
#define ILI9341_CS_Pin_1        GPIO_PIN_4
#define ILI9341_CS_GPIO_Port_1  GPIOA
#define ILI9341_DC_Pin_1        GPIO_PIN_6
#define ILI9341_DC_GPIO_Port_1  GPIOA

#define ILI9341_RES_Pin_2       GPIO_PIN_1
#define ILI9341_RES_GPIO_Port_2 GPIOC
#define ILI9341_CS_Pin_2        GPIO_PIN_0
#define ILI9341_CS_GPIO_Port_2  GPIOC
#define ILI9341_DC_Pin_2        GPIO_PIN_2
#define ILI9341_DC_GPIO_Port_2  GPIOC

// default orientation
#define ILI9341_WIDTH  240
#define ILI9341_HEIGHT 320
#define ILI9341_ROTATION (ILI9341_MADCTL_MX | ILI9341_MADCTL_BGR)

// Color definitions
#define ILI9341_BLACK   0x0000
#define ILI9341_BLUE    0x001F
#define ILI9341_RED     0xF800
#define ILI9341_GREEN   0x07E0
#define ILI9341_CYAN    0x07FF
#define ILI9341_MAGENTA 0xF81F
#define ILI9341_YELLOW  0xFFE0
#define ILI9341_WHITE   0xFFFF
#define ILI9341_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

// call before initializing any SPI devices
void ILI9341_Unselect();

void ILI9341_Init(void);
void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color);
void ILI9341_FillScreen(uint16_t color);
void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ILI9341_MinecraftInit();
void ILI9341_renderMinecraft(float ox);

#endif /* ILI9341_H_ */

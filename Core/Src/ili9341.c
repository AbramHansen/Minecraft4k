/*
 * ili9341.c
 *
 *  Created on: 29 mars 2023
 *      Author: PC-MAGH
 */

#include "ili9341.h"

static void ILI9341_Select() {
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_RESET);
}

void ILI9341_Unselect() {
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port, ILI9341_CS_Pin, GPIO_PIN_SET);
}

static void ILI9341_Reset() {
    HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port, ILI9341_RES_Pin, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port, ILI9341_RES_Pin, GPIO_PIN_SET);
}

static void ILI9341_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&ILI9341_SPI_PORT, &cmd, sizeof(cmd), HAL_MAX_DELAY);

}

static void ILI9341_WriteData(uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);

    // split data in small chunks because HAL can't send more then 64K at once
    while(buff_size > 0) {
        uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
        HAL_SPI_Transmit(&ILI9341_SPI_PORT, buff, chunk_size, HAL_MAX_DELAY);

        buff += chunk_size;
        buff_size -= chunk_size;
    }
}

static void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // column address set
    ILI9341_WriteCommand(0x2A); // CASET
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        ILI9341_WriteData(data, sizeof(data));
    }

    // row address set
    ILI9341_WriteCommand(0x2B); // RASET
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        ILI9341_WriteData(data, sizeof(data));
    }

    // write to RAM
    ILI9341_WriteCommand(0x2C); // RAMWR
}

void ILI9341_Init() {
    ILI9341_Select();
    ILI9341_Reset();

    // command list is based on https://github.com/martnak/STM32-ILI9341

    // SOFTWARE RESET
    ILI9341_WriteCommand(0x01);
    HAL_Delay(1000);

    // POWER CONTROL A
    ILI9341_WriteCommand(0xCB);
    {
        uint8_t data[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // POWER CONTROL B
    ILI9341_WriteCommand(0xCF);
    {
        uint8_t data[] = { 0x00, 0xC1, 0x30 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // DRIVER TIMING CONTROL A
    ILI9341_WriteCommand(0xE8);
    {
        uint8_t data[] = { 0x85, 0x00, 0x78 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // DRIVER TIMING CONTROL B
    ILI9341_WriteCommand(0xEA);
    {
        uint8_t data[] = { 0x00, 0x00 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // POWER ON SEQUENCE CONTROL
    ILI9341_WriteCommand(0xED);
    {
        uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // PUMP RATIO CONTROL
    ILI9341_WriteCommand(0xF7);
    {
        uint8_t data[] = { 0x20 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // POWER CONTROL,VRH[5:0]
    ILI9341_WriteCommand(0xC0);
    {
        uint8_t data[] = { 0x23 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // POWER CONTROL,SAP[2:0];BT[3:0]
    ILI9341_WriteCommand(0xC1);
    {
        uint8_t data[] = { 0x10 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // VCM CONTROL
    ILI9341_WriteCommand(0xC5);
    {
        uint8_t data[] = { 0x3E, 0x28 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // VCM CONTROL 2
    ILI9341_WriteCommand(0xC7);
    {
        uint8_t data[] = { 0x86 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // MEMORY ACCESS CONTROL
    ILI9341_WriteCommand(0x36);
    {
        uint8_t data[] = { 0x48 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // PIXEL FORMAT
    ILI9341_WriteCommand(0x3A);
    {
        uint8_t data[] = { 0x55 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // FRAME RATIO CONTROL, STANDARD RGB COLOR
    ILI9341_WriteCommand(0xB1);
    {
        uint8_t data[] = { 0x00, 0x18 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // DISPLAY FUNCTION CONTROL
    ILI9341_WriteCommand(0xB6);
    {
        uint8_t data[] = { 0x08, 0x82, 0x27 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // 3GAMMA FUNCTION DISABLE
    ILI9341_WriteCommand(0xF2);
    {
        uint8_t data[] = { 0x00 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // GAMMA CURVE SELECTED
    ILI9341_WriteCommand(0x26);
    {
        uint8_t data[] = { 0x01 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // POSITIVE GAMMA CORRECTION
    ILI9341_WriteCommand(0xE0);
    {
        uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                           0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
        ILI9341_WriteData(data, sizeof(data));
    }

    // NEGATIVE GAMMA CORRECTION
    ILI9341_WriteCommand(0xE1);
    {
        uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                           0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
        ILI9341_WriteData(data, sizeof(data));
    }

    // EXIT SLEEP
    ILI9341_WriteCommand(0x11);
    HAL_Delay(120);

    // TURN ON DISPLAY
    ILI9341_WriteCommand(0x29);

    // MADCTL
    ILI9341_WriteCommand(0x36);
    {
        uint8_t data[] = { ILI9341_ROTATION };
        ILI9341_WriteData(data, sizeof(data));
    }

    ILI9341_Unselect();
}

char map[4096]; //16 * 16 * 16

void ILI9341_MinecraftInit() {
    for ( char x = 0; x < 16; x++) {
        for ( char y = 0; y < 16; y++) {
            for ( char z = 0; z < 16; z++) {
                int i = (z * 16 * 16) + (y * 16) + x;
                float yd = (y - 16) * 0.2;
                float zd = (z - 16) * 0.2;
                if (y < 16) {
                    map[i] = (rand() % 5) | 0;
                }
                else {
                    map[i] = 4 | 0;
                }

                if ((double)rand() / RAND_MAX > sqrt(sqrt(yd * yd + zd * zd)) - 0.8)
                    map[i] = 0;
            }
        }
    }
}

void ILI9341_renderMinecraft(float oy) {

    ILI9341_Select();
    ILI9341_SetAddressWindow(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);

    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);

    float xRot = 1.5;
    float yRot = 0.0;

    float ox = 14.5;
    //float oy = 14.5;
    float oz = 14.5;

    float yCos = cos(yRot);
    float ySin = sin(yRot);
    float xCos = cos(xRot);
    float xSin = sin(xRot);

    for (int y = 0; y < ILI9341_HEIGHT; y++) {
        float ___yd = (y - ILI9341_HEIGHT / 2) / (float)ILI9341_WIDTH;
        for (int x = 0; x < ILI9341_WIDTH; x++) {
            float __xd = (x - ILI9341_WIDTH / 2) / (float)ILI9341_WIDTH;
            float __zd = 1;

            float ___xd = __xd * yCos + ___yd * ySin;
            float _xd = ___yd * yCos - __xd * ySin;

            float _yd = ___xd * xCos + __zd * xSin;
            float _zd = __zd * xCos - ___xd * xSin;

            int col = 0;
            int br = 255;
            int ddist = 0;

            float closest = 32;
            for (float d = 0; d < 3; d++) {
                float dimLength = _yd;
                if (d == 1)
                    dimLength = _xd;
                if (d == 2)
                    dimLength = _zd;

                float ll = 1 / (dimLength < 0 ? -dimLength : dimLength);
                float xd = (_yd) * ll;
                float yd = (_xd) * ll;
                float zd = (_zd) * ll;

                float initial = oy - (int)oy;
                if (d == 1)
                    initial = ox - (int)ox;
                if (d == 2)
                    initial = oz - (int)oz;
                if (dimLength > 0)
                    initial = 1 - initial;

                float dist = ll * initial;

                float xp = oy + xd * initial;
                float yp = ox + yd * initial;
                float zp = oz + zd * initial;

                if (dimLength < 0) {
                    if (d == 0)
                        yp--;
                    if (d == 1)
                        xp--;
                    if (d == 2)
                        zp--;
                }

                while (dist < closest) {
                    int tex = map[((int)zp & 15) * 16 * 16 | ((int)yp & 15) * 16 | ((int)xp & 15)];

                    if (tex > 0) {
                        int u = ((int)(xp + zp) * 16) & 15;
                        int v = ((int)(yp * 16) & 15) + 16;
                        if (d == 1) {
                            u = ((int)(xp * 16) & 15);
                            v = ((int)(zp * 16) & 15);
                            if (xd < 0)
                                u += 32;
                        }

                        unsigned int cc;

                        /*switch(tex) {
                        case 2: cc = cc = 0xFFFFFF00; break;
                        case 3: cc = cc = 0xFFFF00FF; break;
                        case 4: cc = cc = 0xFF00FFFF; break;
                        default: cc = 0xFFFFFFFF;
                        }*/

                        switch(tex) {
                        case 2: cc = cc = 0x000000FF; break;
                        case 3: cc = cc = 0x0000FF00; break;
                        case 4: cc = cc = 0x00FF0000; break;
                        default: cc = 0xFFFFFFFF;
                        }

                        if (cc > 0) {
                            col = cc;
                            ddist = 255 - (int)((dist / 32 * 255));
                            br = 255 * (255 - ((int)(d + 2) % 3) * 50) / 255;
                            closest = dist;
                        }
                    }
                    xp += xd;
                    yp += yd;
                    zp += zd;
                    dist += ll;
                }
            }

            char r = ((col >> 16) & 0xff) * br * ddist / (255 * 255);
            char g = ((col >> 8) & 0xff) * br * ddist / (255 * 255);
            char b = ((col) & 0xff) * br * ddist / (255 * 255);

            unsigned short result = 0;
            // Extract and map red component (5 bits)
            result |= ((r >> 3) & 0x1F) << 11;

            // Extract and map green component (6 bits)
            result |= ((g >> 2) & 0x3F) << 5;

            // Extract and map blue component (5 bits)
            result |= (b >> 3) & 0x1F;

            uint8_t data[] = {(char)((result >> 8) & 0xFF), (char)(result & 0xFF)};

            HAL_SPI_Transmit(&ILI9341_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);
        }
    }

    ILI9341_Unselect();
}


void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    // clipping
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
    if((x + w - 1) >= ILI9341_WIDTH) w = ILI9341_WIDTH - x;
    if((y + h - 1) >= ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;

    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, x+w-1, y+h-1);

    uint8_t data[] = { color >> 8, color & 0xFF };
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);
    for(y = h; y > 0; y--) {
        for(x = w; x > 0; x--) {
            HAL_SPI_Transmit(&ILI9341_SPI_PORT, data, sizeof(data), HAL_MAX_DELAY);

        }
    }

    ILI9341_Unselect();
}

//DMA test
/*void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    // clipping
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT)) return;
    if((x + w - 1) >= ILI9341_WIDTH) w = ILI9341_WIDTH - x;
    if((y + h - 1) >= ILI9341_HEIGHT) h = ILI9341_HEIGHT - y;

    ILI9341_Select();
    ILI9341_SetAddressWindow(x, y, x+w-1, y+h-1);

    // Prepare data buffer
    int data_size = 5000;
    uint8_t data[5000];

    for (uint32_t i = 0; i < data_size; i += 2) {
        data[i] = color >> 8;      // Higher 8 bits of color
        data[i + 1] = color & 0xFF;// Lower 8 bits of color
    }

    // Set DC pin to indicate data transmission
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port, ILI9341_DC_Pin, GPIO_PIN_SET);

    // Enable DMA for SPI transmission
    HAL_SPI_Transmit_DMA(&ILI9341_SPI_PORT, data, data_size);

    // Deselect display
    ILI9341_Unselect();
}*/

void ILI9341_FillScreen(uint16_t color) {
    ILI9341_FillRectangle(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, color);
}

void ILI9341_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if((x >= ILI9341_WIDTH) || (y >= ILI9341_HEIGHT))
        return;

    ILI9341_Select();

    ILI9341_SetAddressWindow(x, y, x+1, y+1);
    uint8_t data[] = { color >> 8, color & 0xFF };
    ILI9341_WriteData(data, sizeof(data));

    ILI9341_Unselect();
}

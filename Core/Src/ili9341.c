/*
 * ili9341.c
 *
 *  Created on: 29 mars 2023
 *      Author: PC-MAGH
 */

#include "ili9341.h"

static void ILI9341_Select() {
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port_1, ILI9341_CS_Pin_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port_2, ILI9341_CS_Pin_2, GPIO_PIN_RESET);
}

void ILI9341_Unselect() {
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port_1, ILI9341_CS_Pin_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ILI9341_CS_GPIO_Port_2, ILI9341_CS_Pin_2, GPIO_PIN_SET);
}

static void ILI9341_Reset() {
    HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port_1, ILI9341_RES_Pin_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port_2, ILI9341_RES_Pin_2, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port_1, ILI9341_RES_Pin_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ILI9341_RES_GPIO_Port_2, ILI9341_RES_Pin_2, GPIO_PIN_SET);
}

static void ILI9341_WriteCommand(uint8_t cmd) {
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port_1, ILI9341_DC_Pin_1, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port_2, ILI9341_DC_Pin_2, GPIO_PIN_RESET);
    HAL_SPI_Transmit(&ILI9341_SPI_PORT_1, &cmd, sizeof(cmd), HAL_MAX_DELAY);
    HAL_SPI_Transmit(&ILI9341_SPI_PORT_2, &cmd, sizeof(cmd), HAL_MAX_DELAY);
}

static void ILI9341_WriteData_1(uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port_1, ILI9341_DC_Pin_1, GPIO_PIN_SET);

    // split data in small chunks because HAL can't send more then 64K at once
    while(buff_size > 0) {
        uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
        HAL_SPI_Transmit(&ILI9341_SPI_PORT_1, buff, chunk_size, HAL_MAX_DELAY);

        buff += chunk_size;
        buff_size -= chunk_size;
    }
}

static void ILI9341_WriteData_2(uint8_t* buff, size_t buff_size) {
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port_2, ILI9341_DC_Pin_2, GPIO_PIN_SET);

    // split data in small chunks because HAL can't send more then 64K at once
    while(buff_size > 0) {
        uint16_t chunk_size = buff_size > 32768 ? 32768 : buff_size;
        HAL_SPI_Transmit(&ILI9341_SPI_PORT_2, buff, chunk_size, HAL_MAX_DELAY);

        buff += chunk_size;
        buff_size -= chunk_size;
    }
}

static void ILI9341_SetAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // column address set
    ILI9341_WriteCommand(0x2A); // CASET
    {
        uint8_t data[] = { (x0 >> 8) & 0xFF, x0 & 0xFF, (x1 >> 8) & 0xFF, x1 & 0xFF };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // row address set
    ILI9341_WriteCommand(0x2B); // RASET
    {
        uint8_t data[] = { (y0 >> 8) & 0xFF, y0 & 0xFF, (y1 >> 8) & 0xFF, y1 & 0xFF };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
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
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // POWER CONTROL B
    ILI9341_WriteCommand(0xCF);
    {
        uint8_t data[] = { 0x00, 0xC1, 0x30 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // DRIVER TIMING CONTROL A
    ILI9341_WriteCommand(0xE8);
    {
        uint8_t data[] = { 0x85, 0x00, 0x78 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // DRIVER TIMING CONTROL B
    ILI9341_WriteCommand(0xEA);
    {
        uint8_t data[] = { 0x00, 0x00 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // POWER ON SEQUENCE CONTROL
    ILI9341_WriteCommand(0xED);
    {
        uint8_t data[] = { 0x64, 0x03, 0x12, 0x81 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // PUMP RATIO CONTROL
    ILI9341_WriteCommand(0xF7);
    {
        uint8_t data[] = { 0x20 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // POWER CONTROL,VRH[5:0]
    ILI9341_WriteCommand(0xC0);
    {
        uint8_t data[] = { 0x23 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // POWER CONTROL,SAP[2:0];BT[3:0]
    ILI9341_WriteCommand(0xC1);
    {
        uint8_t data[] = { 0x10 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // VCM CONTROL
    ILI9341_WriteCommand(0xC5);
    {
        uint8_t data[] = { 0x3E, 0x28 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // VCM CONTROL 2
    ILI9341_WriteCommand(0xC7);
    {
        uint8_t data[] = { 0x86 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // MEMORY ACCESS CONTROL
    ILI9341_WriteCommand(0x36);
    {
        uint8_t data[] = { 0x48 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // PIXEL FORMAT
    ILI9341_WriteCommand(0x3A);
    {
        uint8_t data[] = { 0x55 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // FRAME RATIO CONTROL, STANDARD RGB COLOR
    ILI9341_WriteCommand(0xB1);
    {
        uint8_t data[] = { 0x00, 0x18 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // DISPLAY FUNCTION CONTROL
    ILI9341_WriteCommand(0xB6);
    {
        uint8_t data[] = { 0x08, 0x82, 0x27 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // 3GAMMA FUNCTION DISABLE
    ILI9341_WriteCommand(0xF2);
    {
        uint8_t data[] = { 0x00 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // GAMMA CURVE SELECTED
    ILI9341_WriteCommand(0x26);
    {
        uint8_t data[] = { 0x01 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // POSITIVE GAMMA CORRECTION
    ILI9341_WriteCommand(0xE0);
    {
        uint8_t data[] = { 0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
                           0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00 };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
    }

    // NEGATIVE GAMMA CORRECTION
    ILI9341_WriteCommand(0xE1);
    {
        uint8_t data[] = { 0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
                           0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F };
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
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
        ILI9341_WriteData_1(data, sizeof(data));
        ILI9341_WriteData_2(data, sizeof(data));
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

int16_t gyro_x_change;
int16_t gyro_y_change;
int16_t gyro_z_change;

int16_t gyro_x =0;
int16_t gyro_y =0;
int16_t gyro_z =0;

int8_t imu_data[14];

void mpu9250_write_reg(int8_t reg, int8_t data){

      HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

      HAL_SPI_Transmit(&GYRO_SPI_PORT, &reg, 1, 100);

      HAL_SPI_Transmit(&GYRO_SPI_PORT, &data, 1, 100);

      HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

}

void mpu9250_read_reg(int8_t reg, int8_t *data, int8_t len){

      uint8_t temp_data = 0x80|reg;

      HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);

      HAL_SPI_Transmit(&GYRO_SPI_PORT, &temp_data , 1, 100);

      HAL_SPI_Receive(&GYRO_SPI_PORT, data, len, 100);

      HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

}

void readGyro(){
	  mpu9250_read_reg(59, imu_data, sizeof(imu_data));

	  // Split the values up

	  gyro_x_change = (((int16_t)imu_data[8]<<8) + imu_data[9])   + 975;

	  gyro_y_change = (((int16_t)imu_data[10]<<8) + imu_data[11]) + 513;

	  gyro_z_change = (((int16_t)imu_data[12]<<8) + imu_data[13]) + 130;


	  if (-300 >= gyro_x_change || gyro_x_change >= 300)
		  gyro_x += gyro_x_change;

	  if (-300 >= gyro_y_change || gyro_y_change >= 300)
	      gyro_y += gyro_y_change;

	  if (-300 >= gyro_z_change || gyro_z_change >= 300)
		  gyro_z += gyro_z_change;
}

void ILI9341_renderMinecraft(float oy) {

    ILI9341_Select();
    ILI9341_SetAddressWindow(0, 0, ILI9341_WIDTH, ILI9341_HEIGHT);
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port_1, ILI9341_DC_Pin_1, GPIO_PIN_SET);
    HAL_GPIO_WritePin(ILI9341_DC_GPIO_Port_2, ILI9341_DC_Pin_2, GPIO_PIN_SET);

    float xRot = 1.5;// + ((float)gyro_x / 100000);
    float yRot = 0.0;// + ((float)gyro_y / 100000);

    float ox1 = 14.5;
    float oy1 = oy;
    float oz1 = 14.5;

    float ox2 = 14.5;
    float oy2 = oy;
    float oz2 = 14.0;

    float yCos = cos(yRot);
    float ySin = sin(yRot);
    float xCos = cos(xRot);
    float xSin = sin(xRot);

    for (int y = 0; y < ILI9341_HEIGHT; y++) {
        float ___yd = (y - ILI9341_HEIGHT / 2) / (float)ILI9341_WIDTH;

        if(!(y/10))
        	readGyro();

        for (int x = 0; x < ILI9341_WIDTH; x++) {
            float __xd = (x - ILI9341_WIDTH / 2) / (float)ILI9341_WIDTH;
            float __zd = 1;

            float ___xd = __xd * yCos + ___yd * ySin;
            float _xd = ___yd * yCos - __xd * ySin;

            float _yd = ___xd * xCos + __zd * xSin;
            float _zd = __zd * xCos - ___xd * xSin;

            int col1 = 0;
            int br1 = 255;
            int ddist1 = 0;

            int col2 = 0;
            int br2 = 255;
            int ddist2 = 0;

            float closest1 = 32;
            float closest2 = 32;

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

                float initial1 = oy1 - (int)oy1;
                float initial2 = oy2 - (int)oy2;
                if (d == 1) {
                    initial1 = ox1 - (int)ox1;
                	initial2 = ox2 - (int)ox2;
                }
                if (d == 2) {
                    initial1 = oz1 - (int)oz1;
                	initial2 = oz2 - (int)oz2;
                }
                if (dimLength > 0) {
                    initial1 = 1 - initial1;
                    initial2 = 1 - initial2;
                }

                float dist1 = ll * initial1;
                float dist2 = ll * initial2;

                float xp1 = oy1 + xd * initial1;
                float xp2 = oy2 + xd * initial2;
                float yp1 = ox1 + yd * initial1;
                float yp2 = ox2 + yd * initial2;
                float zp1 = oz1 + zd * initial1;
                float zp2 = oz2 + zd * initial2;

                if (dimLength < 0) {
                    if (d == 0) {
                        yp1--;
                        yp2--;
                    }
                    if (d == 1) {
                        xp1--;
                        xp2--;
                    }
                    if (d == 2) {
                        zp1--;
                        zp2--;
                    }
                }

                while (dist1 < closest1) {
                    int tex = map[((int)zp1 & 15) * 16 * 16 | ((int)yp1 & 15) * 16 | ((int)xp1 & 15)];

                    if (tex > 0) {
                        int u = ((int)(xp1 + zp1) * 16) & 15;
                        int v = ((int)(yp1 * 16) & 15) + 16;
                        if (d == 1) {
                            u = ((int)(xp1 * 16) & 15);
                            v = ((int)(zp1 * 16) & 15);
                            if (xd < 0)
                                u += 32;
                        }

                        unsigned int cc;

                        switch(tex) {
                        case 2: cc = cc = 0xFFFFFF00; break;
                        case 3: cc = cc = 0xFFFF00FF; break;
                        case 4: cc = cc = 0xFF00FFFF; break;
                        default: cc = 0xFFFFFFFF;
                        }

                        /*switch(tex) {
                        case 2: cc = cc = 0x000000FF; break;
                        case 3: cc = cc = 0x0000FF00; break;
                        case 4: cc = cc = 0x00FF0000; break;
                        default: cc = 0xFFFFFFFF;
                        }*/

                        if (cc > 0) {
                            col1 = cc;
                            ddist1 = 255 - (int)((dist1 / 32 * 255));
                            br1 = 255 * (255 - ((int)(d + 2) % 3) * 50) / 255;
                            closest1 = dist1;
                        }
                    }
                    xp1 += xd;
                    yp1 += yd;
                    zp1 += zd;
                    dist1 += ll;
                }

                while (dist2 < closest2) {
                    int tex = map[((int)zp2 & 15) * 16 * 16 | ((int)yp2 & 15) * 16 | ((int)xp2 & 15)];

                    if (tex > 0) {
                        int u = ((int)(xp2 + zp2) * 16) & 15;
                        int v = ((int)(yp2 * 16) & 15) + 16;
                        if (d == 1) {
                            u = ((int)(xp2 * 16) & 15);
                            v = ((int)(zp2 * 16) & 15);
                            if (xd < 0)
                                u += 32;
                        }

                        unsigned int cc;

                        switch(tex) {
                        case 2: cc = cc = 0xFFFFFF00; break;
                        case 3: cc = cc = 0xFFFF00FF; break;
                        case 4: cc = cc = 0xFF00FFFF; break;
                        default: cc = 0xFFFFFFFF;
                        }

                        /*switch(tex) {
                        case 2: cc = cc = 0x000000FF; break;
                        case 3: cc = cc = 0x0000FF00; break;
                        case 4: cc = cc = 0x00FF0000; break;
                        default: cc = 0xFFFFFFFF;
                        }*/

                        if (cc > 0) {
                            col2 = cc;
                            ddist2 = 255 - (int)((dist2 / 32 * 255));
                            br2 = 255 * (255 - ((int)(d + 2) % 3) * 50) / 255;
                            closest2 = dist2;
                        }
                    }
                    xp2 += xd;
                    yp2 += yd;
                    zp2 += zd;
                    dist2 += ll;
                }
            }

            char r1 = ((col1 >> 16) & 0xff) * br1 * ddist1 / (255 * 255);
            char g1 = ((col1 >> 8) & 0xff) * br1 * ddist1 / (255 * 255);
            char b1 = ((col1) & 0xff) * br1 * ddist1 / (255 * 255);

            char r2 = ((col2 >> 16) & 0xff) * br2 * ddist2 / (255 * 255);
            char g2 = ((col2 >> 8) & 0xff) * br2 * ddist2 / (255 * 255);
            char b2 = ((col2) & 0xff) * br2 * ddist2 / (255 * 255);

            unsigned short result1 = 0;
            // Extract and map red component (5 bits)
            result1 |= ((r1 >> 3) & 0x1F) << 11;

            // Extract and map green component (6 bits)
            result1 |= ((g1 >> 2) & 0x3F) << 5;

            // Extract and map blue component (5 bits)
            result1 |= (b1 >> 3) & 0x1F;

            unsigned short result2 = 0;
            // Extract and map red component (5 bits)
            result2 |= ((r2 >> 3) & 0x1F) << 11;

            // Extract and map green component (6 bits)
            result2 |= ((g2 >> 2) & 0x3F) << 5;

            // Extract and map blue component (5 bits)
            result2 |= (b2 >> 3) & 0x1F;

            uint8_t data1[] = {(char)((result1 >> 8) & 0xFF), (char)(result1 & 0xFF)};
            uint8_t data2[] = {(char)((result2 >> 8) & 0xFF), (char)(result2 & 0xFF)};

            HAL_SPI_Transmit(&ILI9341_SPI_PORT_1, data1, sizeof(data1), HAL_MAX_DELAY);
            HAL_SPI_Transmit(&ILI9341_SPI_PORT_2, data2, sizeof(data2), HAL_MAX_DELAY);
        }
    }

    ILI9341_Unselect();
}


/*void ILI9341_FillRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
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
}*/

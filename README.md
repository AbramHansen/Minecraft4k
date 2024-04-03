# Ghetto STM32L476RG powered VR

[3D demo code ported from the JavaScript Version of Minecraft4k by Markus Persson](https://jsfiddle.net/uzMPU)

[Software ILI9341 STM32 Driver code](https://www.hackster.io/theembeddedthings/embedded-graphics-display-stm32-and-ili9341-tft-integration-0551bb)

[ILI9341 Display Driver Documentation](http://www.lcdwiki.com/res/MSP2807/ILI9341%20Datasheet.pdf)

## Pages of note:
	how a pixel is sent (16 bit representation): 63
	write cycle sequence: 33-35
	page address set command: 112

## Wiring:
### Display 1:
		PA4 - Chip Select (CS / CSX)
		PA1 - Reset
		PA6 - Data/Command (D/CX)
		PA7 - MOSI (SDA / data)
		PA5 - Serial Clock (SCL)

### Display 2:
		PC0 - Chip Select
		PC1 - Reset
		PC2 - Data/Command
		PC3 - MOSI
		PB10 - Serial Clock

## SPI settings:
	Data Size = 8bits
	MSB First
	Prescaller: 2

#include "SSD1306.h"

typedef enum 
{
	False = 0,
	True  = 1	
}bool_t;

static char SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];
static bool_t Inverted = 0;
int CurrentX = 0;
int CurrentY = 0;
void SSD1306_Init(void)
{
	
	I2C_Init();
	SSD1306_Send_Command(0xAE);
	SSD1306_Send_Command(0xAE); //display off
	SSD1306_Send_Command(0x20); //Set Memory Addressing Mode   
	SSD1306_Send_Command(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	SSD1306_Send_Command(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	SSD1306_Send_Command(0xC8); //Set COM Output Scan Direction
	SSD1306_Send_Command(0x00); //---set low column address
	SSD1306_Send_Command(0x10); //---set high column address
	SSD1306_Send_Command(0x40); //--set start line address
	SSD1306_Send_Command(0x81); //--set contrast control register
	SSD1306_Send_Command(0xFF);
	SSD1306_Send_Command(0xA1); //--set segment re-map 0 to 127
	SSD1306_Send_Command(0xA6); //--set normal display
	SSD1306_Send_Command(0xA8); //--set multiplex ratio(1 to 64)
	SSD1306_Send_Command(0x3F); //
	SSD1306_Send_Command(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	SSD1306_Send_Command(0xD3); //-set display offset
	SSD1306_Send_Command(0x00); //-not offset
	SSD1306_Send_Command(0xD5); //--set display clock divide ratio/oscillator frequency
	SSD1306_Send_Command(0xF0); //--set divide ratio
	SSD1306_Send_Command(0xD9); //--set pre-charge period
	SSD1306_Send_Command(0x22); //
	SSD1306_Send_Command(0xDA); //--set com pins hardware configuration
	SSD1306_Send_Command(0x12);
	SSD1306_Send_Command(0xDB); //--set vcomh
	SSD1306_Send_Command(0x20); //0x20,0.77xVcc
	SSD1306_Send_Command(0x8D); //--set DC-DC enable
	SSD1306_Send_Command(0x14); //
	SSD1306_Send_Command(0xAF); //--turn on SSD1306 panel
	
}


void SSD1306_Send_Command(char command)
{
	char b[2];
	b[0] = 0b00000000;
	b[1] = command;
	I2C_Send(b, SSD1306_Address);
}


void SSD1306_Send_Data(char data)
{
	char b[2];
	b[0] = 0b01000000;
	b[1]= data;
	I2C_Send(b, SSD1306_Address);
}


void SSD1306_Send_Array(char *Array, int size)
{
	for(int i=0; i< size; i++)
	{
		SSD1306_Send_Data(Array[i]);
	}
}


void SSD1306_Fill(SSD1306_COLOR color) 
{
	/* Set memory */
	uint32_t i;

	for(i = 0; i < sizeof(SSD1306_Buffer); i++)
	{
		SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
	}
}

void ssd1306_UpdateScreen(void) 
{
	uint8_t i;
	
	for (i = 0; i < 8; i++) {
		SSD1306_Send_Command(0xB0 + i);
		SSD1306_Send_Command(0x00);
		SSD1306_Send_Command(0x10);
		SSD1306_Send_Command(0x40);
		
		//Fill OLED RAM by Buffer
		SSD1306_Send_Array(&SSD1306_Buffer[SSD1306_WIDTH * i],SSD1306_WIDTH);		
	}
}

void ssd1306_DrawPixel(uint8_t x, uint8_t y,uint8_t color)
{
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) 
	{
		// We are not going to write outside the screen
		return;
	}
	
	// See if the pixel needs to be inverted
	if (Inverted) 
	{
		color = (SSD1306_COLOR)!color;
	}
	
	// We set the right color for the pixel
	if (color == White)
	{
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] |= 1 << (y % 8);
	} 
	else 
	{
		SSD1306_Buffer[x + (y / 8) * SSD1306_WIDTH] &= ~(1 << (y % 8));
	}
}

//
// We want to send 1 char to the screen
// ch => char to write
// Font => Font with which we are going to write
// color => Black or White

char ssd1306_WriteChar(char ch, FontDef Font, SSD1306_COLOR color)
{
	uint32_t i, b, j;
	
	// See if there is still room on this line
	if (SSD1306_WIDTH  <= (CurrentX + Font.FontWidth ) ||
		  SSD1306_HEIGHT <= (CurrentY + Font.FontHeight))
	{
		// There is no more room
		return 0;
	}
	
	// We go through the font
	for (i = 0; i < Font.FontHeight; i++)
	{
		b = Font.data[(ch - 32) * Font.FontHeight + i];
		for (j = 0; j < Font.FontWidth; j++)
		{
			if ((b << j) & 0x8000) 
			{
				ssd1306_DrawPixel(CurrentX + j, (CurrentY + i), (SSD1306_COLOR) color);
			} 
			else 
			{
				ssd1306_DrawPixel(CurrentX + j, (CurrentY + i), (SSD1306_COLOR)!color);
			}
		}
	}
	
	// De huidige positie is nu verplaatst
	CurrentX += Font.FontWidth;
	
	// We geven het geschreven char terug voor validatie
	return ch;
}

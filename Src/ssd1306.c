#include"ssd1306.h"

// Screenbuffer
static uint8_t SSD1306_Buffer[SSD1306_WIDTH * SSD1306_HEIGHT / 8];

// Screen object
static SSD1306_t SSD1306;


//
//  Send a byte to the command register
//
static void ssd1306_WriteCommand(uint8_t command)
{
	I2C1_WriteBytesToAddr (SSD1306_I2C_ADDR, 0x00, &command, 1);
}
//
//	Initialize the oled screen
//
uint8_t ssd1306_Init(void)
{	
	// Wait for the screen to boot
	LL_mDelay(10);

	ssd1306_WriteCommand(0xAE); //display off
	ssd1306_WriteCommand(0x20); //Set Memory Addressing Mode   
	ssd1306_WriteCommand(0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	ssd1306_WriteCommand(0xB0); //Set Page Start Address for Page Addressing Mode,0-7
	ssd1306_WriteCommand(0xC8); //Set COM Output Scan Direction
	ssd1306_WriteCommand(0x00); //---set low column address
	ssd1306_WriteCommand(0x10); //---set high column address
	ssd1306_WriteCommand(0x40); //--set start line address
	ssd1306_WriteCommand(0x81); //--set contrast control register
	ssd1306_WriteCommand(0xFF);
	ssd1306_WriteCommand(0xA1); //--set segment re-map 0 to 127
	ssd1306_WriteCommand(0xA6); //--set normal display
	ssd1306_WriteCommand(0xA8); //--set multiplex ratio(1 to 64)
  //ssd1306_WriteCommand(0x3F); //--- height 64
	ssd1306_WriteCommand(0x1F); // --- height 32
	ssd1306_WriteCommand(0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	ssd1306_WriteCommand(0xD3); //-set display offset
	ssd1306_WriteCommand(0x00); //-not offset
	ssd1306_WriteCommand(0xD5); //--set display clock divide ratio/oscillator frequency
	ssd1306_WriteCommand(0xF0); //--set divide ratio
	ssd1306_WriteCommand(0xD9); //--set pre-charge period
	ssd1306_WriteCommand(0x22); //
	ssd1306_WriteCommand(0xDA); //--set com pins hardware configuration
  //ssd1306_WriteCommand(0x12); // -- height 64
	ssd1306_WriteCommand(0x02); // -- height 32
	ssd1306_WriteCommand(0xDB); //--set vcomh
	ssd1306_WriteCommand(0x20); //0x20,0.77xVcc
	ssd1306_WriteCommand(0x8D); //--set DC-DC enable
	ssd1306_WriteCommand(0x14); //
	ssd1306_WriteCommand(0xAF); //--turn on SSD1306 panel
	
	// Clear screen
	ssd1306_Fill(Black);
	
	// Flush buffer to screen
	ssd1306_UpdateScreen();
	
	// Set default values for screen object
	SSD1306.CurrentX = 0;
	SSD1306.CurrentY = 0;
	
	SSD1306.Initialized = 1;
	
	return 1;
}

//
//  Fill the whole screen with the given color
//
void ssd1306_Fill(SSD1306_COLOR color) 
{
	/* Set memory */
	uint32_t i;

	for(i = 0; i < sizeof(SSD1306_Buffer); i++)
	{
		SSD1306_Buffer[i] = (color == Black) ? 0x00 : 0xFF;
	}
}

//
//  Write the screenbuffer with changed to the screen
//
void ssd1306_UpdateScreen(void) 
{
	uint8_t i;
	
	for (i = 0; i < 8; i++) {
		ssd1306_WriteCommand(0xB0 + i);
		ssd1306_WriteCommand(0x00);
		ssd1306_WriteCommand(0x10);
		I2C1_WriteBytesToAddr ( SSD1306_I2C_ADDR, 0x40, &SSD1306_Buffer[SSD1306_WIDTH * i], SSD1306_WIDTH);
	}
}

//
//	Draw one pixel in the screenbuffer
//	X => X Coordinate
//	Y => Y Coordinate
//	color => Pixel color
//
void ssd1306_DrawPixel(uint8_t x, uint8_t y, SSD1306_COLOR color)
{
	if (x >= SSD1306_WIDTH || y >= SSD1306_HEIGHT) 
	{
		// Don't write outside the buffer
		return;
	}
	
	// Check if pixel should be inverted
	if (SSD1306.Inverted) 
	{
		color = (SSD1306_COLOR)!color;
	}
	
	// Draw in the right color
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
//  Draw 1 char to the screen buffer
//	ch 		=> char om weg te schrijven
//	Font 	=> Font waarmee we gaan schrijven
//	color 	=> Black or White
//


//
//	Position the cursor
//
void ssd1306_SetCursor(uint8_t x, uint8_t y) 
{
	SSD1306.CurrentX = x;
	SSD1306.CurrentY = y;
}

void ssd1306_DrawChar(char ch, const uint8_t font[], uint8_t X, uint8_t Y, SSD1306_COLOR color)
{
    if ((ch < 31) || (ch > 127))
        return;

    uint8_t fOffset, fWidth, fHeight, fBPL, fStart;
    uint8_t* tempChar;

    fOffset = font[0];
    fWidth = font[1];
    fHeight = font[2];
    fBPL = font[3];
    fStart = font[4];


    tempChar = (uint8_t*)&font[((ch - fStart ) * fOffset) + 5]; /* Current Character = Meta + (Character Index * Offset) */


    for (int j = 0; j < fHeight; j++) {
        for (int i = 0; i < fWidth; i++) {
            uint8_t z = tempChar[fBPL * i + ((j & 0xF8) >> 3) + 1]; /* (j & 0xF8) >> 3, increase one by 8-bits */
            uint8_t b = 1 << (j & 0x07);
            if ((z & b) != 0x00)
            	ssd1306_DrawPixel(X + i, Y + j, (SSD1306_COLOR) color);
            else
            	ssd1306_DrawPixel(X + i, Y + j, (SSD1306_COLOR) !color);

        }
    }
}

void ssd1306_DrawText(const char* str, const uint8_t font[], uint8_t X, uint8_t Y,  SSD1306_COLOR color)
{
    uint8_t charWidth; /* Width of character */
    uint8_t fOffset = font[0]; /* Offset of character */
    uint8_t fWidth = font[1]; /* Width of font */
    uint8_t fStart = font[4]; // starting font

    while (*str) {
        ssd1306_DrawChar(*str, font,  X, Y,  color);
        /* Check character width and calculate proper position */
        uint8_t* tempChar = (uint8_t*)&font[((*str - fStart) * fOffset) + 5];
        charWidth = tempChar[0];

        if (charWidth + 2 < fWidth) {
            /* If character width is smaller than font width */
            X += (charWidth + 2);
        }
        else {
            X += fWidth;
        }
        str++;
    }
}
void ssd1306_DrawDirect(uint8_t column, uint8_t page, const uint8_t *tile)
{

	ssd1306_WriteCommand(SSD1306_COLUMNADDR);
	ssd1306_WriteCommand(column);
	ssd1306_WriteCommand(column + 7);

	ssd1306_WriteCommand(SSD1306_PAGEADDR);
	ssd1306_WriteCommand(page);
	ssd1306_WriteCommand(page);

//	ssd1306_writedata(tile, 8);
	I2C1_WriteBytesToAddr ( SSD1306_I2C_ADDR, 0x40,( uint8_t *)  tile, 8);
}

void ssd1306_RestoreFullWindow(void)
{
	ssd1306_WriteCommand(SSD1306_COLUMNADDR);
    ssd1306_WriteCommand(0x00);
    ssd1306_WriteCommand(0x7F);

    ssd1306_WriteCommand(SSD1306_PAGEADDR);
    ssd1306_WriteCommand(0x00);
    ssd1306_WriteCommand(0x07);
}

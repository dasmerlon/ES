// ES-exercise 06                                                //
// Demo to initialize TFT-Display with ST7735R controller,       //
// e.g. joy-it RB-TFT1.8-V2.                          //
// configuration:  4-line serial interface, RGB-order: R-G-B     //


#if defined(__SAM3X8E__)
  #include <DueTimer.h>
  // Objects for configuration of Arduino Due's hardware timers
  DueTimer Timer3;      //object named Timer4 to match Timer4 of MEGAS library
#else
  #include <TimerThree.h>
#endif

#include <SD.h>
#include <SPI.h>
 
//pin declarations
#define SD_CS    4    //display: SD-CS pin
#define TFT_CS     10   //display: Display CS-pin
#define TFT_RST     9   //display: reset
#define TFT_DC      8   //display: Data/Command (D/C)

#if defined(__AVR_ATmega2560__)
 #define SS_SLAVE   53  
 //must be put into output mode; otherwise ATmega could assume 
 //to be set into slave mode but SPI-lib doesn't support this
 //mode. So it breaks SPI-lib. Configured as output, the pin 
 //can be used as normal output.
#endif



#define TFT_DC_HIGH()           digitalWrite(TFT_DC, HIGH)
#define TFT_DC_LOW()            digitalWrite(TFT_DC, LOW)
#define TFT_CS_HIGH()           digitalWrite(TFT_CS, HIGH)
#define TFT_CS_LOW()            digitalWrite(TFT_CS, LOW)
#define SD_CS_HIGH()           digitalWrite(SD_CS, HIGH)
#define SD_CS_LOW()            digitalWrite(SD_CS, LOW)


//SPI-Settings
#define SPI_DEFAULT_FREQ   1e6      ///< Default SPI data clock frequency
SPISettings settingsTFT(SPI_DEFAULT_FREQ, MSBFIRST, SPI_MODE0);


//TFT-area of 128 x 160 (1.8") TFT
const uint8_t FIRST_COL = 2;
const uint8_t FIRST_ROW = 1;
const uint8_t LAST_COL = 129;
const uint8_t LAST_ROW = 160;

//TFT's commands
const uint8_t NOP = 0x00;               // no Operation 
const uint8_t SWRESET = 0x01;           // Software reset                                                                                                                  
const uint8_t SLPOUT = 0x11;            //Sleep out & booster on                                                                                                           
const uint8_t DISPOFF = 0x28;           //Display off                                                                                                                          
const uint8_t DISPON = 0x29;            //Display on                                                                                                                       
const uint8_t CASET = 0x2A;             //Column adress set                                                                                                        
const uint8_t RASET = 0x2B;             //Row adress set                                                                                                           
const uint8_t RAMWR = 0x2C;             //Memory write                                                                                                             
const uint8_t MADCTL = 0x36;            //Memory Data Access Control                                                                                                       
const uint8_t COLMOD = 0x3A;            //RGB-format, 12/16/18bit                                                                                                          
const uint8_t INVOFF = 0x20;            // Display inversion off                                                                                                           
const uint8_t INVON = 0x21;             // Display inversion on                                                                                                    
const uint8_t INVCTR = 0xB4;            //Display Inversion mode control                                                                                                   
const uint8_t NORON = 0x13;             //Partial off (Normal)                                                                                                     
                                                                                                                                                                           
const uint8_t PWCTR1 = 0xC0;            //Power Control 1                                                                                                                  
const uint8_t PWCTR2 = 0xC1;            //Power Control 2                                                                                                                  
const uint8_t PWCTR3 = 0xC2;            //Power Control 3                                                                                                                  
const uint8_t PWCTR4 = 0xC3;            //Power Control 4                                                                                                                  
const uint8_t PWCTR5 = 0xC4;            //Power Control 5
const uint8_t VMCTR1 = 0xC5;            //VCOM Voltage setting

uint8_t buffer [128][40];
uint8_t buffer_write_offset = 0;
uint8_t line_spacing = 10;
uint8_t delay_time = 0;
uint8_t bgColor = 0x03; // blue
uint8_t fgColor = 0xFF; // white
unsigned char font[95][6] =
{
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, // space
{ 0x00, 0x00, 0x5F, 0x00, 0x00, 0x00 }, // !
{ 0x00, 0x07, 0x00, 0x07, 0x00, 0x00 }, // "
{ 0x14, 0x7F, 0x14, 0x7F, 0x14, 0x00 }, // #
{ 0x24, 0x2A, 0x7F, 0x2A, 0x12, 0x00 }, // $
{ 0x23, 0x13, 0x08, 0x64, 0x62, 0x00 }, // %
{ 0x36, 0x49, 0x55, 0x22, 0x50, 0x00 }, // &
{ 0x00, 0x00, 0x07, 0x00, 0x00, 0x00 }, // '
{ 0x00, 0x1C, 0x22, 0x41, 0x00, 0x00 }, // (
{ 0x00, 0x41, 0x22, 0x1C, 0x00, 0x00 }, // )
{ 0x0A, 0x04, 0x1F, 0x04, 0x0A, 0x00 }, // *
{ 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00 }, // +
{ 0x00, 0x50, 0x30, 0x00, 0x00, 0x00 }, // ,
{ 0x08, 0x08, 0x08, 0x08, 0x08, 0x00 }, // -
{ 0x00, 0x60, 0x60, 0x00, 0x00, 0x00 }, // .
{ 0x20, 0x10, 0x08, 0x04, 0x02, 0x00 }, // slash
{ 0x3E, 0x51, 0x49, 0x45, 0x3E, 0x00 }, // 0
{ 0x00, 0x42, 0x7F, 0x40, 0x00, 0x00 }, // 1
{ 0x42, 0x61, 0x51, 0x49, 0x46, 0x00 }, // 2
{ 0x21, 0x41, 0x45, 0x4B, 0x31, 0x00 }, // 3
{ 0x18, 0x14, 0x12, 0x7F, 0x10, 0x00 }, // 4
{ 0x27, 0x45, 0x45, 0x45, 0x39, 0x00 }, // 5
{ 0x3C, 0x4A, 0x49, 0x49, 0x30, 0x00 }, // 6
{ 0x03, 0x71, 0x09, 0x05, 0x03, 0x00 }, // 7
{ 0x36, 0x49, 0x49, 0x49, 0x36, 0x00 }, // 8
{ 0x06, 0x49, 0x49, 0x29, 0x1E, 0x00 }, // 9
{ 0x00, 0x36, 0x36, 0x00, 0x00, 0x00 }, // :
{ 0x00, 0x56, 0x36, 0x00, 0x00, 0x00 }, // ;
{ 0x08, 0x14, 0x22, 0x41, 0x00, 0x00 }, // <
{ 0x14, 0x14, 0x14, 0x14, 0x14, 0x00 }, // =
{ 0x00, 0x41, 0x22, 0x14, 0x08, 0x00 }, // >
{ 0x02, 0x01, 0x51, 0x09, 0x06, 0x00 }, // ?
{ 0x32, 0x49, 0x79, 0x41, 0x3E, 0x00 }, // @
{ 0x7E, 0x11, 0x11, 0x11, 0x7E, 0x00 }, // A
{ 0x7F, 0x49, 0x49, 0x49, 0x36, 0x00 }, // B
{ 0x3E, 0x41, 0x41, 0x41, 0x22, 0x00 }, // C
{ 0x7F, 0x41, 0x41, 0x41, 0x3E, 0x00 }, // D
{ 0x7F, 0x49, 0x49, 0x49, 0x41, 0x00 }, // E
{ 0x7F, 0x09, 0x09, 0x09, 0x01, 0x00 }, // F
{ 0x3E, 0x41, 0x41, 0x49, 0x7A, 0x00 }, // G
{ 0x7F, 0x08, 0x08, 0x08, 0x7F, 0x00 }, // H
{ 0x00, 0x41, 0x7F, 0x41, 0x00, 0x00 }, // I
{ 0x20, 0x40, 0x41, 0x3F, 0x01, 0x00 }, // J
{ 0x7F, 0x08, 0x14, 0x22, 0x41, 0x00 }, // K
{ 0x7F, 0x40, 0x40, 0x40, 0x40, 0x00 }, // L
{ 0x7F, 0x02, 0x0C, 0x02, 0x7F, 0x00 }, // M
{ 0x7F, 0x04, 0x08, 0x10, 0x7F, 0x00 }, // N
{ 0x3E, 0x41, 0x41, 0x41, 0x3E, 0x00 }, // O
{ 0x7F, 0x09, 0x09, 0x09, 0x06, 0x00 }, // P
{ 0x3E, 0x41, 0x51, 0x21, 0x5E, 0x00 }, // Q
{ 0x7F, 0x09, 0x19, 0x29, 0x46, 0x00 }, // R
{ 0x26, 0x49, 0x49, 0x49, 0x32, 0x00 }, // S
{ 0x01, 0x01, 0x7F, 0x01, 0x01, 0x00 }, // T
{ 0x3F, 0x40, 0x40, 0x40, 0x3F, 0x00 }, // U
{ 0x1F, 0x20, 0x40, 0x20, 0x1F, 0x00 }, // V
{ 0x3F, 0x40, 0x38, 0x40, 0x3F, 0x00 }, // W
{ 0x63, 0x14, 0x08, 0x14, 0x63, 0x00 }, // X
{ 0x07, 0x08, 0x70, 0x08, 0x07, 0x00 }, // Y
{ 0x61, 0x51, 0x49, 0x45, 0x43, 0x00 }, // Z
{ 0x00, 0x7F, 0x41, 0x41, 0x00, 0x00 }, // [
{ 0x02, 0x04, 0x08, 0x10, 0x20, 0x00 }, // backslash
{ 0x00, 0x41, 0x41, 0x7F, 0x00, 0x00 }, // ]
{ 0x04, 0x02, 0x01, 0x02, 0x04, 0x00 }, // ^
{ 0x40, 0x40, 0x40, 0x40, 0x40, 0x00 }, // _
{ 0x00, 0x01, 0x02, 0x04, 0x00, 0x00 }, // `
{ 0x20, 0x54, 0x54, 0x54, 0x78, 0x00 }, // a
{ 0x7F, 0x48, 0x44, 0x44, 0x38, 0x00 }, // b
{ 0x38, 0x44, 0x44, 0x44, 0x20, 0x00 }, // c
{ 0x38, 0x44, 0x44, 0x48, 0x7F, 0x00 }, // d
{ 0x38, 0x54, 0x54, 0x54, 0x18, 0x00 }, // e
{ 0x08, 0x7E, 0x09, 0x01, 0x02, 0x00 }, // f
{ 0x08, 0x54, 0x54, 0x54, 0x3C, 0x00 }, // g
{ 0x7F, 0x08, 0x04, 0x04, 0x78, 0x00 }, // h
{ 0x00, 0x48, 0x7D, 0x40, 0x00, 0x00 }, // i
{ 0x20, 0x40, 0x44, 0x3D, 0x00, 0x00 }, // j
{ 0x7F, 0x10, 0x28, 0x44, 0x00, 0x00 }, // k
{ 0x00, 0x41, 0x7F, 0x40, 0x00, 0x00 }, // l
{ 0x7C, 0x04, 0x78, 0x04, 0x78, 0x00 }, // m
{ 0x7C, 0x08, 0x04, 0x04, 0x78, 0x00 }, // n
{ 0x38, 0x44, 0x44, 0x44, 0x38, 0x00 }, // o
{ 0x7C, 0x14, 0x14, 0x14, 0x08, 0x00 }, // p
{ 0x08, 0x14, 0x14, 0x18, 0x7C, 0x00 }, // q
{ 0x7C, 0x08, 0x04, 0x04, 0x08, 0x00 }, // r
{ 0x48, 0x54, 0x54, 0x54, 0x20, 0x00 }, // s
{ 0x04, 0x3F, 0x44, 0x40, 0x20, 0x00 }, // t
{ 0x3C, 0x40, 0x40, 0x20, 0x7C, 0x00 }, // u
{ 0x1C, 0x20, 0x40, 0x20, 0x1C, 0x00 }, // v
{ 0x3C, 0x40, 0x30, 0x40, 0x3C, 0x00 }, // w
{ 0x44, 0x28, 0x10, 0x28, 0x44, 0x00 }, // x
{ 0x0C, 0x50, 0x50, 0x50, 0x3C, 0x00 }, // y
{ 0x44, 0x64, 0x54, 0x4C, 0x44, 0x00 }, // z
{ 0x00, 0x08, 0x36, 0x41, 0x00, 0x00 }, // {
{ 0x00, 0x00, 0x7F, 0x00, 0x00, 0x00 }, // |
{ 0x00, 0x41, 0x36, 0x08, 0x00, 0x00 }, // }
{ 0x10, 0x08, 0x08, 0x10, 0x08, 0x00 } // ~
};

char ch_buffer[80] = "";

#define INPUT_LENGTH 127
char input_buffer[INPUT_LENGTH] = { 0 };
uint8_t buffer_pos = 0;

void TFTwriteCommand(uint8_t cmd){
    TFT_DC_LOW();
     SPI.transfer(cmd);
    TFT_DC_HIGH();
}

void TFTwrite_saCommand(uint8_t cmd)
{
  SPI.beginTransaction(settingsTFT);
  TFT_CS_LOW();
    TFT_DC_LOW();
     SPI.transfer(cmd);
    TFT_DC_HIGH();
  TFT_CS_HIGH();
  SPI.endTransaction();
}

void TFTwriteWindow(uint8_t xs, uint8_t xe, uint8_t ys, uint8_t ye) 
{
        //test weather parameters stay within address ranges;  should be implemented//
        TFTwriteCommand(NOP);   //normally not neccessary; but if not, the first command after eg. SD-access will be ignored (here: CASET)
        TFTwriteCommand(CASET);
        SPI.transfer(0x00); SPI.transfer(xs);
        SPI.transfer(0x00); SPI.transfer(xe);
        TFTwriteCommand(RASET);
        SPI.transfer(0x00); SPI.transfer(ys);
        SPI.transfer(0x00); SPI.transfer(ye);
}

void TFTinit(void) 
{
      SPI.beginTransaction(settingsTFT);
      TFT_CS_LOW();

        TFTwriteCommand(SWRESET);
        delay(120);                                     //mandatory delay
        TFTwriteCommand(SLPOUT);        //turn off sleep mode.
        delay(120);
        TFTwriteCommand(PWCTR1);
         SPI.transfer(0xA2);
         SPI.transfer(0x02);
         SPI.transfer(0x84);
        TFTwriteCommand(PWCTR4);
         SPI.transfer(0x8A);
         SPI.transfer(0x2A);
        TFTwriteCommand(PWCTR5);
         SPI.transfer(0x8A);
         SPI.transfer(0xEE);
        TFTwriteCommand(VMCTR1);
         SPI.transfer(0x0E);
        TFTwriteCommand(MADCTL);
         SPI.transfer(0x08);

        //RGB-format
        TFTwriteCommand(COLMOD);        //color mode
         SPI.transfer(0x55); //16-bit/pixel; high nibble don't care

        TFTwriteCommand(CASET);
         SPI.transfer(0x00); SPI.transfer(FIRST_COL);
         SPI.transfer(0x00); SPI.transfer(LAST_COL);
        TFTwriteCommand(RASET);
         SPI.transfer(0x00); SPI.transfer(FIRST_ROW);
         SPI.transfer(0x00); SPI.transfer(LAST_ROW);

        TFTwriteCommand(NORON);
        TFTwriteCommand(DISPON);

      TFT_CS_HIGH();
      SD_CS_HIGH();
      SPI.endTransaction();
}

void ClearBuffer()
{
  for (uint8_t x = 0; x < 128; x++) 
  {
    for (uint8_t y = 0; y < 40; y++)
    {
      buffer[x][y] = bgColor;
    }
  }
}

void SetPixel(uint8_t x, uint8_t y, uint8_t value)
{ 
   buffer[y][x] = value;
}

void FillArea(uint8_t x_start, uint8_t x_end, uint8_t y_start, uint8_t y_end, uint8_t value)
{
   for (uint8_t x = x_start; x < x_end; x++) 
   {
      for (uint8_t y = y_start; y < y_end; y++)
      {
   SetPixel(x,y,value);
      }
  }
}

void TransferBuffer(uint8_t x_offset, uint8_t columns_to_transfer, uint8_t y_offset, uint8_t rows_to_transfer)
{
  for (uint8_t y = 0; y < columns_to_transfer; y++) 
  {
    SPI.beginTransaction(settingsTFT);
    TFT_CS_LOW();
    TFTwriteWindow(FIRST_COL + y_offset, FIRST_COL + y_offset + rows_to_transfer, y + x_offset, y + x_offset);
    TFTwriteCommand(RAMWR);
    
    //for (uint8_t x = 0; x < 128; x++)
    for (uint8_t x = 0; x < y_offset + rows_to_transfer; x++)
    {
      if(buffer[x][y] == -1)
      {
      }
      else
      {
   //  Split Up into Parts
   uint8_t r8 = buffer[x][y] >> 5;
   uint8_t g8 = (buffer[x][y] << 3) >> 5;
   uint8_t b8 = (buffer[x][y] << 6) >> 6;

   //Convert to 565 Format
   uint16_t r16 = r8 << 13;
   uint16_t g16 = g8 << 8;
   uint16_t b16 = b8 << 3;
   //  RGB to Hex
   uint16_t HexColor = b16 | g16 | r16;
   SPI.transfer(HexColor>>8);
   SPI.transfer(HexColor);
      }
    }
    TFT_CS_HIGH();
    SPI.endTransaction();
  }
}

int get_bit(uint8_t hex, int i) {
    return (hex >> i) & 1;
}

void printChar(uint8_t x_s, uint8_t y_s, char c, uint8_t bg_col, uint8_t char_col)
{
  //  x and y (0,0) is the bottom left coords 
  
  int ascii = c - 32;
  for(uint8_t i = 0; i < 6; i++)
    {
      for(uint8_t j = 0; j < 8; j++)
      {
   if(x_s + i >= 160) 
   {
      Serial.print("Char exceeding buffer");
      return;
   } 
   if(get_bit(font[ascii][i], j)) SetPixel(x_s + i,y_s - j, char_col);
      }
    }
}

void printString(uint8_t x, uint8_t y, char *c_str, uint8_t bg_col, uint8_t char_col)
{
   buffer_write_offset = x;
   for(int i = 0; i < strlen(c_str); i++)
   {
      ClearBuffer();
      printChar(0,y, c_str[i], bg_col, char_col);
      TransferBuffer(buffer_write_offset, 6, y-8, 10);
      buffer_write_offset += 6;
      
      if(buffer_write_offset >= 160)
      {
   Serial.print("Too long!");
   break;
      }
   }
} 

void StudentIdDemo()
{
   FillBGColor();
   char f_name[25] = "Felix Swimmer"; // length 18 chars
   char f_ID[25] = "7162123";
   uint8_t x_start = (160 - (strlen(f_name)*6))/2;
   uint8_t y_start = 128/2 + line_spacing;
   
   printString(x_start, y_start, f_name, bgColor, fgColor);
   x_start = (160 - strlen(f_ID)*6)/2;
   printString(x_start, y_start - line_spacing, f_ID, bgColor, fgColor);
   
   
  
   FillBGColor();
   char h_name[25] = "Hassan Chahrour";
   char h_ID[25] =  "1234567";
   x_start = (160 - (strlen(h_name)*6))/2;
   y_start = 128/2 + line_spacing;
    
  printString(x_start, y_start, h_name, bgColor, fgColor);
   x_start = (160 - (strlen(h_ID)*6))/2;
  printString(x_start, y_start - line_spacing, h_ID, bgColor, fgColor);
  
  
   // Clear the full screen
}

void Bar(uint8_t x_s, uint8_t y_s, char c, uint8_t diameter, uint8_t bg_col, uint8_t char_col)
{
  if(c == '|')
  {
    for(uint8_t y = y_s-1; y > y_s - diameter; y--)
    {
      SetPixel(x_s,y,char_col);
    }
  }
  else if(c == '/')
  {
    uint8_t x = x_s + diameter;
    for(uint8_t y = y_s + diameter-1; y > y_s; y--)
    {
      x--;
      SetPixel(x, y-diameter, char_col);
    }
  }
  else if(c == '-')
  {
    for(uint8_t x = x_s; x < x_s + diameter; x++)
    {
      SetPixel(x,y_s,char_col);
    }
  }
  else if(c == '\\')
  {
    uint8_t x = x_s;
    for(uint8_t y = y_s; y < y_s + diameter; y++)
    {
      x--;
      SetPixel(x+diameter, y-diameter, char_col);
    }
  }
}

void RotatingBar () //TODO: do on a timer, not recursion
{
  uint8_t tmp_buffer_offset = buffer_write_offset;
  
  uint8_t center_x = 20;
  uint8_t center_y = 64;
  uint8_t radius = 6;
  uint8_t diameter = radius*2;
  buffer_write_offset = 60; //start in the middle of the second buffer
  
  
  FillArea(center_x - radius, center_x + radius, center_y - radius, center_y + radius, 0x00);
  Bar(center_x, center_y+radius, '|', diameter, bgColor, fgColor);
  TransferBuffer(buffer_write_offset, 40, 0, 128);

  
  FillArea(center_x - radius, center_x + radius, center_y - radius, center_y + radius, 0x00);
  Bar(center_x-radius, center_y+radius, '/', diameter, bgColor, fgColor);
  TransferBuffer(buffer_write_offset, 40, 0, 128);
 
  
  FillArea(center_x - radius, center_x + radius, center_y - radius, center_y + radius, 0x00);
  Bar(center_x-radius, center_y, '-', diameter, bgColor, fgColor);
  TransferBuffer(buffer_write_offset, 40, 0, 128);
  
  
  FillArea(center_x - radius, center_x + radius, center_y - radius, center_y + radius, 0x00);
  Bar(center_x-radius, center_y+radius, '\\', diameter, bgColor, fgColor);
  TransferBuffer(buffer_write_offset, 40, 0, 128);
  
  buffer_write_offset = tmp_buffer_offset;
  //Timer3.restart();
}


void Wipe()
{
  FillArea(0,40,0,128, fgColor);
  for(uint8_t x=0; x<160;x++)
  {
    TransferBuffer(x, 1, 0, 128);
     
  }

  FillArea(0,40,0,128, bgColor);
   for(uint8_t x=0; x<160;x++)
  {
    TransferBuffer(x, 1, 0, 128);
      
  }
}

void InitDebugScreen()
{
  //alle buffer teile haben andere farben zum debuggen
  FillArea(0, 40, 0, 128, 0xC0); //Rot
  TransferBuffer(0, 40, 0, 128);
  FillArea(0, 40, 0, 128, 0xFC); //Gelb
  TransferBuffer(40, 40, 0, 128);
  FillArea(0, 40, 0, 128, 0x1C); //Grün
  TransferBuffer(80, 40, 0, 128);
  FillArea(0, 40, 0, 128, 0x93); //hell blau
  TransferBuffer(120, 40, 0, 128);
}
void FillBGColor()
{
  FillArea(0, 40, 0, 128, bgColor);
  TransferBuffer(0, 40, 0, 128);
  TransferBuffer(40, 40, 0, 128);
  TransferBuffer(80, 40, 0, 128);
  TransferBuffer(120, 40, 0, 128);
}

void InitSDCard()
{
   SD.begin(SD_CS);
}

void demo()
{
   //InitDebugScreen();
   delay_time = 100; //100ms delay between rotate bar refreshes
   /*Timer3.initialize(); //10hz refresh
   Timer3.setPeriod(delay_time * 4);
   Timer3.attachInterrupt(RotatingBar);*/
   FillBGColor();
   
 
}

void setup() {
  // set pin-modes
  pinMode(TFT_RST, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_CS, OUTPUT);
  #if defined(__AVR_ATmega2560__)  
     pinMode(SS_SLAVE, OUTPUT); 
  #endif

  // set inactive levels
  digitalWrite(TFT_RST, HIGH);
  digitalWrite(TFT_DC, HIGH);
  digitalWrite(TFT_CS, HIGH);

  // initialize serial port 0
  Serial.begin(9600);
  Serial.println("Exercise init TFT template\n");

  // initialize SPI:
  // several devices: multiple SPI.begin(nn_CS) possible
  SPI.begin();
  delay(10);

  //power-on-reset of Display
  digitalWrite(TFT_RST, HIGH);
  delay(100);
  digitalWrite(TFT_RST, LOW);
  delay(100);
  digitalWrite(TFT_RST, HIGH);
  delay(100);

  TFTinit();
  Serial.println("Display Initialized");
  delay(100);

  //clear display
    uint8_t xs = FIRST_COL;
    uint8_t xe = LAST_COL;
    uint8_t ys = FIRST_ROW;
    uint8_t ye = LAST_ROW;
  uint16_t time = millis();
    SPI.beginTransaction(settingsTFT);
    TFT_CS_LOW();
    TFTwriteWindow(xs, xe, ys, ye);
    TFTwriteCommand(RAMWR);  //assign background-color to every element of writewindow
        for (uint16_t i=0; i<(xe+1-xs)*(ye+1-ys); i++) {
            SPI.transfer(0xFF);
            SPI.transfer(0xFF);}
    TFT_CS_HIGH();
    SPI.endTransaction();
  time = millis() - time;
  
  Serial.print("time consumption of clear-display: "); Serial.print(time, DEC); Serial.println(" ms");
  
   //demo();
      //configuration of timer3 (@ 2Hz, 5E5us) and //attatch ISR
  #if defined(__SAM3X8E__)
    //if (Timer3.configure(2, StudentIdDemo)) {; } //DUE-Timer has been configured, but has not been started
    //else {  // timer configuration fails...
      //Serial.println("ERROR: Failed to configure debouncing timer!");
    //}
  #else
    Timer3.initialize(5e5);    //Mega-Timer has benn initilized T = 5e5us (f = 2Hz)
    Timer3.attachInterrupt(StudentIdDemo);    //attachInterrupt starts Timer, if not intended here, stopp timer!
    Timer3.stop();
  #endif
  
  //Timer3.start();    //normally done on demand by application

   InitSDCard();
   FillBGColor();
   help();
   
   Serial.println();
   Serial.print("Awaiting Command: ");
}

void help()
{
   Serial.println();
   Serial.println("//////////////////////////////////////");
   Serial.println("HELP COMMANDS:");
   Serial.println("'help' / '?' : Shows all possible commands");
   Serial.println("'Clear' : Clears the buffer and refreshes the display");
   Serial.println("'RBar' : Runs the rotating bar demo");
   Serial.println("'SID' : Runs the student demo with ID");
   Serial.println("'Stop' : Stops all demos");
   Serial.println("'ls <dir name>' : Shows all files that are within the directory");
   Serial.println("'exist <file name>' : Tells you if the file exists");
   Serial.println("'Serial <file name>' : Outputs the file into the serial monitor");
   Serial.println("'LCD <file name>' : Outputs the file onto the LCD Display");
   Serial.println("//////////////////////////////////////");
   Serial.println();
}

void StopDemo()
{
   Timer3.stop();
}

void ListDirectory(File dir, int numTabs)
{
   while(true) 
   {
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       // return to the first file in the directory
       dir.rewindDirectory();
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       ListDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
   }
}
void DoesFileExist(char* filename)
{
   SD_CS_HIGH();
   if(SD.exists(filename))
   {
      Serial.println("//////////////////////////////////////");
      Serial.println();
      Serial.print(filename);
      Serial.print(" exists!");
      Serial.println("//////////////////////////////////////");
   }
   else
   {
      Serial.println("//////////////////////////////////////");
      Serial.println();
      Serial.print(filename);
      Serial.print(" does not exist!");
      Serial.println("//////////////////////////////////////");
   }
   SD_CS_LOW();
}
void OutputFileToSerial(char* filename)
{
   SD_CS_HIGH();
   File file = SD.open(filename);
   if(strstr(filename, ".txt"))
   {
      Serial.print("Output of ");
      Serial.print(filename);
      Serial.println(":");
      int currentBit = 0;
      while(currentBit != -1)
      {
   currentBit = file.read();
   char c = currentBit;
   Serial.print(c);
      }
      Serial.println();
   }
   else if(strstr(filename, ".img"))
   {
      Serial.print("Output of ");
      Serial.print(filename);
      Serial.println(":");
      
      uint8_t width = 0;
      char currentChar[1];
      while(true)  //break after comma to find width
      {
   currentChar[0] = file.read();
   if(currentChar[0] == ',') break;
   width = atoi(currentChar) + width * 10;
      }
      //break after newline
      while(currentChar[0] != '\n') currentChar[0] = file.read(); 
      int currentBit = 0;
      uint8_t currentWidth = 0;
      while(currentBit != -1)
      {
   currentBit = file.read();
   char c = currentBit; 
   if(c == ',') continue;
   Serial.print(c);
   Serial.print(" ");
   if(currentWidth == width-1) 
   {
      Serial.println();
      currentWidth = 0;
   }
   currentWidth++;
      }
      Serial.println();
   }
   else Serial.println("Invalid File Type!");
   SD_CS_LOW();
}
void OutputFileToLCD(char* filename)
{
   char text[16];
   SD_CS_HIGH();
   File file = SD.open(filename);
   if(strstr(filename, ".txt"))
   {
      int currentBit = 0;
      int index = 0;
      while(currentBit != -1)
      {
   currentBit = file.read();
   char c = currentBit;
   text[index] = c;
   index++;
      }
      printString(0, 60, text, bgColor, fgColor);
   }
   else if(strstr(filename, ".img"))
   {
      uint8_t width = 0;
      uint8_t height = 0;
      char currentChar[1];
      while(true)  //break after comma to find width
      {
   currentChar[0] = file.read();
   if(currentChar[0] == ',') break;
   width = atoi(currentChar) + width * 10;
      }
      while(true)  //break after new line to find height
      {
   currentChar[0] = file.read();
   if(currentChar[0] == '\n') break;
   height = atoi(currentChar) + height * 10;
      }
      int currentBit = 0;
      
      //  TODO: split into multiple buffers if too wide
      
      for(int y = 0; y < 40; y++)
      {
   for(int x = 0; x < 40;)
   {
      for(int skip = 0; skip < width-40;)
      {
         currentBit = file.read();
         char c = currentBit; 
         if(c == ',') continue;
         skip++;
      }
      currentBit = file.read();
      char c = currentBit; 
      if(c == '0') SetPixel(x, y, bgColor);
      else if(c == '1') SetPixel(x, y, fgColor);
      else SetPixel(x, y, 0xFF);
      x++;
   }
      }
      /*
      uint8_t x = 0;
      uint8_t y = 0;
      while(currentBit != -1)
      {
   currentBit = file.read();
   char c = currentBit; 
   if(c == ',') continue;
   if(c == '0') SetPixel(x, y, bgColor);
   else if(c == '1') SetPixel(x, y, fgColor);
   else SetPixel(x, y, 0xFF);
   
   if(x == 39) 
   {
      x = 0;
      y++;
   }
   x++;
      }*/
      // find center to position to
      TransferBuffer(50,40, 50, 40);
   }
   else Serial.println("Invalid File Type!");
   SD_CS_LOW();
}

//  PARSER

void stringToLower(char *str)
{
   for (size_t i = 0; i < strlen(str); ++i) 
   {
      str[i] = tolower((unsigned char) str[i]);
   }
}

int extract(int from, int to, char *str, char *subString)
{
  int i=0, j=0;
  //get the length of the string.
  int length = strlen(str);
  
  if( from > length || from < 0 ){
    printf("The index 'from' is invalid\n");
    return 1;
  }
  if( to > length ){
    printf("The index 'to' is invalid\n");
    return 1;
  }  
  for( i = from, j = 0; i <= to; i++, j++){
    subString[j] = str[i];
  }  
  return 0;  
}

void ParseInputs()
{
   char ch = Serial.read(); //read char
   Serial.write(ch); //echo to serial monitor
   
   if ((ch != 0x0A) && (ch != 0x0B) && (ch != 0x0C) && (ch != 0x0D)) 
   {
      input_buffer[buffer_pos++] = ch;
      if (buffer_pos == INPUT_LENGTH) 
      {
   buffer_pos = 0;
   Serial.println("WARNING: Input buffer overflow!");
      }
   }
   else 
   {
      input_buffer[buffer_pos] = '\0';
      
      stringToLower(input_buffer);
      if(strstr(input_buffer, "help") || strstr(input_buffer, "?"))
      {
   help();
      }
      else if(strstr(input_buffer, "clear"))
      {
   StopDemo();
   FillBGColor();
      }
      else if(strstr(input_buffer, "rbar"))
      {
   StopDemo();
   FillBGColor(); // von mir hinzugefügt
     
   //Timer3.attachInterrupt(RotatingBar);
   Timer3.configure(1, RotatingBar) ;
   Timer3.start();
      }
      else if(strstr(input_buffer, "sid"))
      {
   //Timer3.attachInterrupt(StudentIdDemo);
   Timer3.configure(1, StudentIdDemo) ;
   Timer3.start();
      }
      else if(strstr(input_buffer, "stop"))
      {
   StopDemo();
      }
      else if(strstr(input_buffer, "ls"))
      {
   SD_CS_HIGH();
   ListDirectory(SD.open("/"), 0);
   SD_CS_LOW();
      }
      else if(strstr(input_buffer, "exist"))
      {
   str_cut(input_buffer, 6);
   DoesFileExist(input_buffer);
      }
      else if(strstr(input_buffer, "serial"))
      {
   str_cut(input_buffer, 7);
   OutputFileToSerial(input_buffer);
      }
      else if(strstr(input_buffer, "lcd"))
      {
   str_cut(input_buffer, 4);
   OutputFileToLCD(input_buffer);
      }
      else 
      {
   Serial.println("Invalid Command, try again!");
      }
      buffer_pos = 0;
      ClearInputBuffer();
      input_buffer[buffer_pos] = '\0';
      Serial.print("Awaiting Command: ");
      }
}
void ClearInputBuffer()
{
    for(int i = 0; i < INPUT_LENGTH; i++)
    {
   input_buffer[i] = '0';
    }
}

void str_cut(char *str, int begin)
{
   // TODO: issue is that strings are copied over too long, and clearing doesnt seem to help (test with exist)
    char copy[INPUT_LENGTH];
    for(int i = 0; i < strlen(str); i++)
    {
   copy[i] = str[i];
    }
    for(int i = 0; i < INPUT_LENGTH - begin; i++)
    {
   str[i] = copy[i+begin];
    }
}

void loop() 
{
   while (Serial.available()) ParseInputs();
}   

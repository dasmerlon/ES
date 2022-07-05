// ES-exercise 06                                                //
// Demo to initialize TFT-Display with ST7735R controller,       //
// e.g. joy-it RB-TFT1.8-V2.                          //
// configuration:  4-line serial interface, RGB-order: R-G-B     //

#include <SPI.h>
 
//pin declarations
#define TFT_CS     10   //display: CS-pin
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


//global variables
uint8_t invState = 0;


void TFTwriteCommand(uint8_t cmd){
    TFT_DC_LOW();
     SPI.transfer(cmd);
    TFT_DC_HIGH();
}

void TFTwrite_saCommand(uint8_t cmd){
  SPI.beginTransaction(settingsTFT);
  TFT_CS_LOW();
    TFT_DC_LOW();
     SPI.transfer(cmd);
    TFT_DC_HIGH();
  TFT_CS_HIGH();
  SPI.endTransaction();
}

void TFTwriteWindow(uint8_t xs, uint8_t xe, uint8_t ys, uint8_t ye) {
        //test weather parameters stay within address ranges;  should be implemented//
        TFTwriteCommand(NOP);   //normally not neccessary; but if not, the first command after eg. SD-access will be ignored (here: CASET)
        TFTwriteCommand(CASET);
        SPI.transfer(0x00); SPI.transfer(xs);
        SPI.transfer(0x00); SPI.transfer(xe);
        TFTwriteCommand(RASET);
        SPI.transfer(0x00); SPI.transfer(ys);
        SPI.transfer(0x00); SPI.transfer(ye);
}

void TFTinit(void) {
        //minimal configuration: only settings which are different from Reset Default Value
        //or not affected by HW/SW-reset
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
         SPI.transfer(0x0E);                    //VCOM = -0.775V

        //Memory Data Access Control D7/D6/D5/D4/D3/D2/D1/D0
        //                                                       MY/MX/MV/ML/RGB/MH/-/-
        // MY- Row Address Order; ‘0’ =Increment, (Top to Bottom)
        // MX- Column Address Order; ‘0’ =Increment, (Left to Right)
        // MV- Row/Column Exchange; '0’ = Normal,
        // ML- Scan Address Order; ‘0’ =Decrement,(LCD refresh Top to Bottom)
        //RGB - '0'= RGB color fill order
        // MH - '0'= LCD horizontal refresh left to right
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
      SPI.endTransaction();
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

  Serial.println("\nSetup finished\n");

  demo();
}

uint8_t buffer [160][128];

void SetPixel(uint8_t x, uint8_t y, uint8_t value)
{
    buffer[x][y] = value;
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

void TransferBuffer()
{
  for (uint8_t y = 0; y < 161; y++) 
  {
    SPI.beginTransaction(settingsTFT);
    TFT_CS_LOW();
    TFTwriteWindow(FIRST_COL, LAST_COL, y, y);
    TFTwriteCommand(RAMWR);
    
    for (uint8_t x = 0; x < 128; x++)
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
      //Serial.println(buffer[x][y], HEX);
      SPI.transfer(HexColor>>8);
      SPI.transfer(HexColor);
    }
    TFT_CS_HIGH();
    SPI.endTransaction();
    }
}

 void writeCharIntoBuffer(uint8_t x_s, uint8_t y_s, char c, uint8_t diameter, uint8_t bg_col, uint8_t char_col)
 {
  //  x and y is the top left coords

  //clear in diameter 
  //FillArea(x_s, x_s+diameter, y_s, y_s+diameter, bg_col);
  //TransferBuffer();
  
  //  Write Char into Buffer
  if(c == '|')
  {
    for(uint8_t y = y_s; y > y_s - diameter; y--)
    {
      SetPixel(y,x_s,char_col); //TODO, change pxiel coords`?
    }
  }
  else if(c == '/')
  {
    for(uint8_t xy = x_s+diameter; xy > x_s; xy--)
    {
      SetPixel(xy, xy, char_col);
    }
    
  }
  else if(c == '-')
  {
    for(uint8_t x = x_s; x < x_s + diameter; x++)
    {
      SetPixel(y_s,x,char_col);
    }
  }
  else if(c == '\\')
  {
    uint8_t y = y_s;
    for(uint8_t x = x_s + diameter; x > x_s - diameter; x--)
    {
      y++;
      SetPixel(y, x, char_col);
    }
  }
  
  // determine size of character
  // clear area we are writing on
  //  Write area
  // write into buffer
 }

 void rotateBar (){
  uint8_t center_x = 80;
  uint8_t center_y = 64;
  uint8_t radius = 7;
  uint8_t delay_time = 20;
  //TODO change center!
  writeCharIntoBuffer(center_x, center_y, '|', radius*2+1, 0x00, 0xFF);
  TransferBuffer();
  delay(20);
  writeCharIntoBuffer(center_x, center_y, '/', radius*2+1, 0x00, 0xFF);
  TransferBuffer();
  delay(20);
  writeCharIntoBuffer(center_x, center_y, '-', radius*2+1, 0x00, 0xFF);
  TransferBuffer();
  delay(20);
  writeCharIntoBuffer(center_x, center_y, '\\', radius*2+1, 0x00, 0xFF);
  TransferBuffer();
  delay(20);
  
  }
void demo()
{
  uint8_t fgColor = 0x03;//white 
  
  uint8_t bgColor = 0x1C;// black
  FillArea(0,160, 0, 128, bgColor);
  TransferBuffer();
/*
  for(uint8_t x=0; x<160;x++)
  {
    
    FillArea(x,x,0,128, fgColor);
    TransferBuffer();
    delay(20);  
  }

    for(uint8_t x=0; x<160;x++)
  {
    Serial.println("bin hier");
    FillArea(x,x,0,128, bgColor);
    TransferBuffer();
    delay(20);  
  }*/
  rotateBar();
}

void loop() {
  // delay(1000);
  // (invState) ? TFTwrite_saCommand(INVON): TFTwrite_saCommand(INVOFF);
  // invState = !invState;
}  

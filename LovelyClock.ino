#include <SPI.h>
#include <Wire.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <DS3231.h>

#include <Encoder.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16

#define HOUR_POS_X    5
#define HOUR_POS_Y    30
#define MIN_POS_X     30    
#define MIN_POS_Y     30
#define DATE_POS_X    0   
#define DATE_POS_Y    0
#define MON_POS_X    
#define MON_POS_Y
#define DAY_POS_X    
#define DAY_POS_Y
#define YEAR_POS_X    
#define YEAR_POS_Y

static const unsigned char PROGMEM logo_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };


DS3231 Clock;  
const int buttonPin = 4;
Encoder Enc(3, 2);
bool h12;
bool PM;
bool century = false;
int hour, minute, second;
bool setMode = 0;
int curButtonState, prevButtonState = 1;

const char *days[] = {"", "PAZ", "PZT", "SAL", "CAR", "PER", "CUM", "CTS"};
const char *mons[] = {"", "OCA", "SUB", "MAR", "NIS", "MAY", "HAZ", "TEM", "AGU", "EYL", "EKI", "KAS", "ARA"};

void displayDateTimeTemp();

void setup() {
  //Serial init
  Serial.begin(9600);

  //Display Init
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();  
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0, 32);
  display.println(F("Meltem' s Clock"));  
  display.display();      // Show initial text  

  //Clock Init
  Clock.setClockMode(false);
  
  //Button init
  pinMode(buttonPin, INPUT);
 
  delay(1000);  
}

void loop() {
  displayDateTimeTemp();  
  if (setMode == true) {
    display.fillRect(HOUR_POS_X+5*4*4+4*4, HOUR_POS_Y, 5*4, 7*4, INVERSE);
  }
  display.display();

  curButtonState = digitalRead(buttonPin);
  if (prevButtonState == 1 && curButtonState == 0){
    setMode = !setMode;
  }
  prevButtonState = curButtonState;  
  
  delay(50);
}

void displayDateTimeTemp()
{
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(DATE_POS_X, DATE_POS_Y);    
  display.print(Clock.getDate());
  display.print(" ");
  display.print(mons[Clock.getMonth(century)]);
  display.print(" ");
  display.print(days[Clock.getDoW()]);
  display.print("\n20");
  display.print(Clock.getYear());
  
  display.setTextSize(2);
  display.setCursor(80, 0);
  display.print(round(Clock.getTemperature()));display.write(9);display.print("C"); 
  

  display.setTextSize(4);
  display.setCursor(HOUR_POS_X, HOUR_POS_Y);  
  hour = Clock.getHour(h12, PM);
  if (hour < 10)
    display.print(0);
  display.print(hour);
  
  display.print(" ");
  
  minute = Clock.getMinute();  
  if (minute < 10)
    display.print(0);
  display.print(minute);
    
  second = Clock.getSecond();
  if (second % 2 == 0) {    
    display.setTextSize(2);
    display.setCursor(59, 35);      
    display.write(3);
  }
}

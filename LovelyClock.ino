/*
 * IINCLUDES
 */
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DS3231.h>
#include <Encoder.h>

/*
 * MACROS
 */
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)

#define HOUR_POS_X    5
#define HOUR_POS_Y    34
#define MINUTE_POS_X  80
#define MINUTE_POS_Y  HOUR_POS_Y
#define SECOND_POS_X  59
#define SECOND_POS_Y  40
#define DATE_POS_X    2   
#define DATE_POS_Y    2
#define MONTH_POS_X   20   
#define MONTH_POS_Y   3
#define DOW_POS_X     50   
#define DOW_POS_Y     3
#define YEAR_POS_X    DATE_POS_X
#define YEAR_POS_Y    15
#define TEMP_POS_X    80   
#define TEMP_POS_Y    2
#define X_POS_X       TEMP_POS_X + 30
#define X_POS_Y       TEMP_POS_Y

/*
 * CONSTANTS
 */
const char *days[] = {"", "PAZ", "PZT", "SAL", "CAR", "PER", "CUM", "CTS"};
const char *mons[] = {"", "OCA", "SUB", "MAR", "NIS", "MAY", "HAZ", "TEM", "AGU", "EYL", "EKI", "KAS", "ARA"};
const char *messages[] = {"", "Message1", "Message2", "Message3", "Message4", "Message5", "Message6", "Message7"};
 
/*
 * GLOBAL VARIABLES
 */
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DS3231 Clock;  
Encoder Enc(3, 2);
const int buttonPin = 4;
bool h12;
bool PM;
bool century = false;

// State machine states
enum DeviceState { NORMAL, SET_SELECTION, SET_VALUE };
DeviceState deviceState = NORMAL;
long menuSelection;
int setValue;

/*
 * FUNCTION DECLARATIONS
 */
void displayHour(int hour);
void displayMinute(int minute);
void displaySecond(int second);
void displayDate(int date);
void displayDoW(int dow);
void displayYear(int year);
void displayMessage();
void displayDateTime();
void displayDateTimeWithMessage();
void displayTemp();
void displayX();
bool isButtonPressed();
long getEncoderDif();
int boundVal(int val, int maxVal, int minVal);
void normalState();
void setSelectionState();
void setValueState();

/*
 * FUNCTION DEFINITIONS
 */

// Initial setup function
void setup() {
  //Serial init
  Serial.begin(9600);

  //Clock Init
  Clock.setClockMode(false);
  
  //Button init
  pinMode(buttonPin, INPUT);
 
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
  display.println(F("M' s Clock"));  
  display.display();      // Show initial text  
  
  delay(1000);  
}

// Main loop
void loop() {
  switch (deviceState) {
    case NORMAL:
      normalState();
      break;
    case SET_SELECTION:
      setSelectionState();
      break;
    case SET_VALUE:
      setValueState();
      break;      
  }
  display.display();    
  delay(50);
}

// Prints the hour value at the position predefined by macros
void displayHour(int hour)
{
  display.setTextSize(4);
  display.setCursor(HOUR_POS_X, HOUR_POS_Y);  
  if (hour < 10)
    display.print(0);
  display.print(hour);  
}

// Prints the minute value at the position predefined by macros
void displayMinute(int minute)
{
  display.setTextSize(4);
  display.setCursor(MINUTE_POS_X, MINUTE_POS_Y);  
  if (minute < 10)
    display.print(0);
  display.print(minute);  
}

// Prints the second indicator at the position predefined by macros
void displaySecond(int second)
{  
  if (second % 2 == 0) {    
    display.setTextSize(2);
    display.setCursor(SECOND_POS_X, SECOND_POS_Y);      
    display.write(3);
  }  
}

void displayDate(int date)
{
  display.setTextSize(1);
  display.setCursor(DATE_POS_X, DATE_POS_Y);    
  display.print(date);  
}

void displayMonth(int month)
{
  display.setTextSize(1);
  display.setCursor(MONTH_POS_X, MONTH_POS_Y);    
  display.print(mons[month]);
}

void displayDoW(int dow)
{
  display.setTextSize(1);
  display.setCursor(DOW_POS_X, DOW_POS_Y); 
  display.print(days[dow]);  
}

void displayYear(int year)
{
  display.setTextSize(1);
  display.setCursor(YEAR_POS_X, YEAR_POS_Y); 
  display.print("20");
  display.print(year);  
}

void displayMessage(int messageNumber)
{
  display.setTextSize(2);
  display.setCursor(2, 2);
  display.print(messages[messageNumber]);  
}

// Displays month, day of month, day of week, year, hour, minute, and second indicator on OLED at the positions defined by macros
void displayDateTime()
{  
  displayDate(Clock.getDate());
  displayMonth(Clock.getMonth(century));  
  displayDoW(Clock.getDoW());
  displayYear(Clock.getYear());    
  displayHour(Clock.getHour(h12, PM));  
  displayMinute(Clock.getMinute());  
  displaySecond(Clock.getSecond());
}

void displayDateTimeWithMessage()
{
  int minute = Clock.getMinute();  
  int dow = Clock.getDoW();
  
  if (minute == 0) {    
    displayHour(Clock.getHour(h12, PM));  
    displayMinute(minute);  
    displaySecond(Clock.getSecond());
    displayMessage(dow);              
  }
  else {
    displayDate(Clock.getDate());
    displayMonth(Clock.getMonth(century));  
    displayDoW(dow);
    displayYear(Clock.getYear());    
    displayHour(Clock.getHour(h12, PM));  
    displayMinute(minute);  
    displaySecond(Clock.getSecond());
    displayTemp();
  }
}

// Displays temperature value on OLED at position defined by macro
void displayTemp()
{
  display.setTextSize(2);
  display.setCursor(TEMP_POS_X, TEMP_POS_Y);
  display.print(round(Clock.getTemperature()));
  display.write(9);
  display.print("C");     
}

// Displays a close mark (X) when navigating menu at temperature value position 
void displayX()
{
  display.setTextSize(2);
  display.setCursor(X_POS_X, X_POS_Y);
  display.print("X");     
}

// Detects button pin transition from 1 to 0 by polling. An external pull-up resistor is connected to the button pin.
bool isButtonPressed()
{
  static int curButtonState, prevButtonState = 1;
  
  curButtonState = digitalRead(buttonPin);
  if (prevButtonState == 1 && curButtonState == 0) {
    prevButtonState = curButtonState;
    return true;
  }
  prevButtonState = curButtonState;
  return false;    
}

// Gets encoder difference from last reading. For each tick my encoder value changes by 2. Therefore I added some logic to convert it to change by 1. 
long getEncoderDiff()
{  
  static long lastEncPos, curEncPos, dif;

  curEncPos = Enc.read();
  dif = curEncPos - lastEncPos;
  if (abs(dif) >= 2 && dif % 2 == 0) {    
    lastEncPos = curEncPos;
    return dif/2;
  }
  return 0;  
}

// Bounds the value between min and max values in a circular manner. 
int boundVal(int val, int minVal, int maxVal)
{
  if (val > maxVal)
    return minVal;
  else if (val < minVal)
    return maxVal;
  else
    return val;    
}

void normalState()
{  
  display.clearDisplay();
  displayDateTimeWithMessage();  

  if (isButtonPressed())
    deviceState = SET_SELECTION;
}

void setSelectionState()
{  
  menuSelection += getEncoderDiff();
  menuSelection = boundVal(menuSelection, 0, 6);
    
  display.clearDisplay();
  displayDateTime();
  displayX();

  switch (menuSelection) {
    case 0: //X
      display.drawRect(X_POS_X - 2, X_POS_Y - 2, 14, 18, WHITE);      
      break;
    case 1: //Date
      display.drawRect(DATE_POS_X - 2, DATE_POS_Y - 2, 14, 11, WHITE);      
      break;
    case 2: //Month
      display.drawRect(MONTH_POS_X - 2, MONTH_POS_Y - 2, 22, 11, WHITE);      
      break;
    case 3: //Day of Week
      display.drawRect(DOW_POS_X - 2, DOW_POS_Y - 2, 22, 11, WHITE);      
      break;
    case 4: //Year
      display.drawRect(YEAR_POS_X - 2, YEAR_POS_Y - 2, 28, 11, WHITE);      
      break;
    case 5: //Hour     
      display.drawRect(HOUR_POS_X - 2, HOUR_POS_Y - 2, 48, 32, WHITE);      
      break;
    case 6: //Minute
      display.drawRect(MINUTE_POS_X - 2, MINUTE_POS_Y - 2, 48, 32, WHITE);      
      break;
  }
    
  if (isButtonPressed()) {
    if (menuSelection == 0) {
      deviceState = NORMAL;
    }
    else {      
      deviceState = SET_VALUE;
      switch (menuSelection) {        
        case 1:
          setValue = Clock.getDate();
          break;
        case 2:
          setValue = Clock.getMonth(century);
          break;
        case 3:
          setValue = Clock.getDoW();
          break;
        case 4:
          setValue = Clock.getYear();
          break;
        case 5:
          setValue = Clock.getHour(h12, PM);
          break;
        case 6:         
          setValue = Clock.getMinute();
          break;
      }
    }
  }
    
}

void setValueState()
{  
  display.clearDisplay();
  displayDateTime();
  displayX();

  setValue += getEncoderDiff();
    
  switch (menuSelection) {    
    case 1: //Date
      setValue = boundVal(setValue, 1, 31);
      display.fillRect(DATE_POS_X - 2, DATE_POS_Y - 2, 14, 11, BLACK);
      displayDate(setValue);
      display.fillRect(DATE_POS_X - 2, DATE_POS_Y - 2, 14, 11, INVERSE);
      break;
    case 2: //Month
      setValue = boundVal(setValue, 1, 12);
      display.fillRect(MONTH_POS_X - 2, MONTH_POS_Y - 2, 22, 11, BLACK);
      displayMonth(setValue);    
      display.fillRect(MONTH_POS_X - 2, MONTH_POS_Y - 2, 22, 11, INVERSE);
      break;
    case 3://Day of Week
      setValue = boundVal(setValue, 1, 7);
      display.fillRect(DOW_POS_X - 2, DOW_POS_Y - 2, 22, 11, BLACK);
      displayDoW(setValue);    
      display.fillRect(DOW_POS_X - 2, DOW_POS_Y - 2, 22, 11, INVERSE);
      break;
    case 4://Year
      setValue = boundVal(setValue, 0, 99);
      display.fillRect(YEAR_POS_X - 2, YEAR_POS_Y - 2, 28, 11, BLACK);
      displayYear(setValue);    
      display.fillRect(YEAR_POS_X - 2, YEAR_POS_Y - 2, 28, 11, INVERSE);
      break;
    case 5: //Hour
      setValue = boundVal(setValue, 0, 23);
      display.fillRect(HOUR_POS_X - 2, HOUR_POS_Y - 2, 48, 32, BLACK);
      displayHour(setValue);
      display.fillRect(HOUR_POS_X - 2, HOUR_POS_Y - 2, 48, 32, INVERSE);
      break;
    case 6: //Minute
      setValue = boundVal(setValue, 0, 59);
      display.fillRect(MINUTE_POS_X - 2, MINUTE_POS_Y - 2, 48, 32, BLACK);
      displayMinute(setValue);
      display.fillRect(MINUTE_POS_X - 2, MINUTE_POS_Y - 2, 48, 32, INVERSE);
      break;    
  }
  
  if (isButtonPressed()) {
    switch (menuSelection) {      
      case 1:
        Clock.setDate(setValue);
        break;
      case 2:
        Clock.setMonth(setValue);
        break;
      case 3:
        Clock.setDoW(setValue);
        break;
      case 4:
        Clock.setYear(setValue);
        break;
      case 5:
        Clock.setHour(setValue);
        break;
      case 6:
        Clock.setMinute(setValue);
        break;      
    }
    deviceState = SET_SELECTION;
  }
}

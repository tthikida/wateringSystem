#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <RTClib.h>
#include <EEPROM.h>

//save flag


// Create objecsts
DHT dht(9, DHT11);
RTC_DS3231 rtc;
LiquidCrystal_I2C lcd(0x27,16,2);


// EEPROM
const int hourAddress           = 0;
const int minuteAddress         = 1;
const int durationMinuteAddress = 2;
const int durationSecondAddress = 3;


// Pins
const int back         = 2;
const int select       = 3;
const int up           = 4;
const int right        = 5;
const int down         = 6;
const int left         = 7;
const int ledPin       = 13;
const int relay        = 8;
const int tempHumidity = 9;


// Buttons and Menu Index
const int mainMenuIndexMax          = 2;
const int setWaterSchedule1IndexMax = 6;

int upState        = 0;
int downState      = 0;
int leftState      = 0;
int rightState     = 0;
int backState      = 0;
int selectState    = 0;
int mainMenuIndex  = 0;
int setWaterSchedule1Index = 0;

String currentMenu    = "MainMenu";
String mainMenu       = "MainMenu";
String waterSchedule1 = "WaterSchedule1";


// Time, Date
int rtc_year;
int rtc_month; 
int rtc_day; 
int rtc_hour;
int rtc_minute;
int rtc_second;


// Water Schedule 1
int ws1TimeOnHour        = 5;
int ws1TimeOnMinute      = 0;
int ws1DurationMinute    = 0;
int ws1DurationSecond    = 30;
int setWS1DurationMinute = 0;
int setWS1DurationSecond = 30;


// Timing
const int     oneSecondInterval    = 1;
unsigned long debounceDelay        = 150;
unsigned long previousSeconds      = 0;
unsigned long currentSeconds       = 0;
unsigned long previousSecondsTimer = 0;
unsigned long currentSecondsTimer  = 0;


float humidity;
float temperature;


const bool ON        = HIGH;
const bool OFF       = LOW;
const bool ON_RELAY  = LOW;
const bool OFF_RELAY = HIGH;
bool waterSchedule1IsOn;
bool timeToWater;
bool delayTimer;




// Setup
void BlinkForSetup(){
  for(int i = 3; i >= 0; i--){
    digitalWrite(ledPin, ON);
    delay(250);
    digitalWrite(ledPin, OFF);
    delay(50);
  }
  for(int i = 4; i >= 0; i--){
    digitalWrite(ledPin, ON);
    delay(50);
    digitalWrite(ledPin, OFF);
    delay(50);
  }
}
void SetupPins(){
  pinMode(back,  INPUT);
  pinMode(select,  INPUT);
  pinMode(up,    INPUT);
  pinMode(down,  INPUT);
  pinMode(left,  INPUT);
  pinMode(right, INPUT);
  pinMode(tempHumidity, INPUT);

  pinMode(ledPin, OUTPUT);
  pinMode(relay,  OUTPUT);

  digitalWrite(relay, OFF_RELAY);
}
void MoreSetupStuff(){
  currentMenu          = "MainMenu";
  waterSchedule1IsOn   = true;
  timeToWater          = false;
  delayTimer           = false;
  ws1TimeOnHour        = EEPROM.read(hourAddress);
  ws1TimeOnMinute      = EEPROM.read(minuteAddress);
  setWS1DurationMinute = EEPROM.read(durationMinuteAddress);
  setWS1DurationSecond = EEPROM.read(durationSecondAddress);
  ws1DurationMinute    = setWS1DurationMinute;
  ws1DurationSecond    = setWS1DurationSecond;
  

  dht.begin();
  rtc.begin();

  // Uncomment the folOFFing line to set the initial time and date
  // rtc.adjust(DateTime(2023,9,11,22,22,0));


  }
void SetupLCD(){
  lcd.init();
  lcd.backlight();
  lcd.clear();
  PrintTitle();
}


// Print Fuctions
void PrintTitle(){
  lcd.setCursor(0,0);
  lcd.print("\\  \\ Bonsai \\ \\");
  lcd.setCursor(0,1);
  lcd.print("Watering System");
}
void PrintMainMenu(int mainMenuIndex){
  currentMenu = "MainMenu";
  lcd.clear();
  switch(mainMenuIndex){
    case 0:
      PrintTitle();
      break;
    case 1:
      PrintDate();
      PrintTime();
      PrintTemperatureHumidity();
      break;
    case 2:
      PrintWaterPlant(1, "OFF");
      break;
    default:
      Serial.println("default mainMenuIndex");
  }
}
void PrintDate(){
  lcd.setCursor(0,0);
  lcd.print(rtc_day);
  lcd.print(".");
  lcd.print(rtc_month);
  lcd.print("/");
  lcd.print(rtc_year);
}
void PrintTime(){
  lcd.setCursor(0,1);
  if(rtc_hour<10){
    lcd.print("0");
  }
  lcd.print(rtc_hour);
  lcd.print(":");
  if(rtc_minute<10){
    lcd.print("0");
  }
  lcd.print(rtc_minute);
  lcd.print(":");
  if(rtc_second<10){
    lcd.print("0");
  }
  lcd.print(rtc_second);
}
void PrintTemperatureHumidity(){
  humidity    = dht.readHumidity();
  temperature = dht.readTemperature();
  
  lcd.setCursor(10,0);
  lcd.print("T:");
  lcd.print(temperature);
  lcd.setCursor(14,0);
  lcd.print((char)223);
  lcd.print("C");
  lcd.setCursor(10,1);
  lcd.print("H:");
  lcd.print(humidity);
  lcd.setCursor(14,1);
  lcd.print("%  ");
} 
void PrintWaterPlant(int plantNum, String onOffString){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Water");
  // lcd.print(plantNum);
  lcd.setCursor(10,0);
  lcd.print("Menu->");
  lcd.setCursor(0,1);
  lcd.print(onOffString);
  lcd.setCursor(3,1);
  lcd.print("<-Manual Ctl");
}
void PrintSetWaterSchedule1(){
  currentMenu = waterSchedule1;
  lcd.clear();
  switch(setWaterSchedule1Index){
    case 0:
      //ON
      lcd.setCursor(0,0);
      lcd.print("TimeON  ");
      lcd.setCursor(9,0);
      if(ws1TimeOnHour<10){
          lcd.print("0");
        }
      lcd.print(ws1TimeOnHour);
      lcd.print(":");
      if(ws1TimeOnMinute<10){
          lcd.print("0");
        }
      lcd.print(ws1TimeOnMinute);
      //OFF
      lcd.setCursor(0,1);
      lcd.print("Duration ");
      lcd.setCursor(9,1);
      if(ws1DurationMinute<10){
          lcd.print("0");
        }
      lcd.print(ws1DurationMinute);
      lcd.print("m ");
      if(ws1DurationSecond<10){
          lcd.print("0");
        }
      lcd.print(ws1DurationSecond);
      lcd.print("s");
      break;
    case 1:
      lcd.setCursor(0,0);
      lcd.print("Schedule On/Off?");
      lcd.setCursor(0,1);
      if(waterSchedule1IsOn){
        lcd.print("OFF       ->  ON");
      } else {
        lcd.print("OFF  <-       ON");
      }
      break;
    case 2:
      SetWaterScheduleTopCursor();
      lcd.print("Time ON - Hour");
      lcd.setCursor(4,1);
      lcd.print("<- ");
      if(ws1TimeOnHour<10){
        lcd.print("0");
      }
      lcd.print(ws1TimeOnHour);
      lcd.print(" ->");
      break;
    case 3:
      SetWaterScheduleTopCursor();
      lcd.print("Time ON - Min");
      lcd.setCursor(4,1);
      lcd.print("<- ");
      if(ws1TimeOnMinute<10){
        lcd.print("0");
      }
      lcd.print(ws1TimeOnMinute);
      lcd.print(" ->");
      break;
    case 4:
      SetWaterScheduleTopCursor();
      lcd.print("Duration - Min");
      lcd.setCursor(4,1);
      lcd.print("<- ");
      if(setWS1DurationMinute<10){
        lcd.print("0");
      }
      lcd.print(setWS1DurationMinute);
      lcd.print(" ->");
      break;
    case 5:
    SetWaterScheduleTopCursor();
      lcd.print("Duration - Sec");
      lcd.setCursor(4,1);
      lcd.print("<- ");
      if(setWS1DurationSecond<10){
        lcd.print("0");
      }
      lcd.print(setWS1DurationSecond);
      lcd.print(" ->");
      break;
    case 6:
      lcd.setCursor(0,0);
      lcd.print("Save Schedule?");
      lcd.setCursor(10,1);
      lcd.print("Save->  ");
      break;
    
    default:
      Serial.print("default - setWaterSchedule1Index - PrintSetWaterSchedule1");
  }
}




// Button Logic
void CheckButtonInput(){
  upState     = digitalRead(up);
  downState   = digitalRead(down);
  leftState   = digitalRead(left);
  rightState  = digitalRead(right);
  backState   = digitalRead(back);
  selectState = digitalRead(select);
  delay(debounceDelay);
}
void MenuButtonsFunctions(){
  MainMenuButtons();
  SetWaterSchedule1Buttons();
}
void MainMenuButtons(){
  if(currentMenu != mainMenu){return;}

  // UP
  if (upState == ON) {
    digitalWrite(ledPin, ON);
    mainMenuIndex++;
    if(mainMenuIndex > mainMenuIndexMax){
      mainMenuIndex = 0;
    }
    PrintMainMenu(mainMenuIndex);
  } 
  // DOWN
  else if (downState == ON) {
    digitalWrite(ledPin, ON);
    mainMenuIndex--;
    if(mainMenuIndex < 0){
      mainMenuIndex = mainMenuIndexMax;
    }
    PrintMainMenu(mainMenuIndex);
  }
  // LEFT
  else if (leftState == ON) {
    digitalWrite(ledPin, ON);
    Serial.println("Left - Main Menu");
    switch(mainMenuIndex){
      case 2:
        Relay_OnOff(relay, 1);
        break;
      default:
        Serial.print("default - Left - mainMenuIndex");
    }
  }
  // RIGHT
  else if (rightState == ON) {
    digitalWrite(ledPin, ON);
    switch(mainMenuIndex){
      case 0:
      //Title
        Serial.print("MainMenu - index 0 - Select button pressed");
        break;
      case 1:
      //TimeAndDate
        Serial.print("MainMenu - index 1 - Select button pressed");
        break;
      case 2:
      //WaterPlant1
        Serial.print("MainMenu - index 2 - Select button pressed");
        setWaterSchedule1Index = 0;
        PrintSetWaterSchedule1();
        break;
      default:
        Serial.print("default case - MainMenuButtons - Select");
    }
  }
  // BACK
  else if (backState == ON) {
    digitalWrite(ledPin, ON);
    Serial.println("Back - Main Menu");
    mainMenuIndex = 1;
    PrintMainMenu(1);
  }
  // SELECT
  else if (selectState == ON) {
    digitalWrite(ledPin, ON);
    Serial.println("Select - Main Menu");
    switch(mainMenuIndex){
      case 0:
      //Title
        Serial.print("MainMenu - index 0 - Select button pressed");
        break;
      case 1:
      //TimeAndDate
        Serial.print("MainMenu - index 1 - Select button pressed");
        // PrintSetTimeDateMenu();
        break;
      case 2:
      //WaterPlant1
        Serial.print("MainMenu - index 2 - Select button pressed");
        setWaterSchedule1Index = 0;
        PrintSetWaterSchedule1();
        break;
      case 3:
      //WaterPlant2
        Serial.print("MainMenu - index 3 - Select button pressed");
        // PrintSetWaterSchedule2());
        break;
      default:
        Serial.print("default case - MainMenuButtons - Select");
    }
  }
  else {
    digitalWrite(ledPin, OFF);
  }
}
void SetWaterSchedule1Buttons(){
  if(currentMenu != waterSchedule1){return;}
  // UP
  if(upState == ON) {
    setWaterSchedule1Index++;
    if(setWaterSchedule1Index > setWaterSchedule1IndexMax){
      setWaterSchedule1Index = 0;
    }
    PrintSetWaterSchedule1();
  } 
  // DOWN
  else if (downState == ON) {
    setWaterSchedule1Index--;
    if(setWaterSchedule1Index < 0){
      setWaterSchedule1Index = setWaterSchedule1IndexMax;
    }
    PrintSetWaterSchedule1();
  }
  // LEFT
  else if(leftState == ON) {
    switch(setWaterSchedule1Index){
      case 1:
        waterSchedule1IsOn = false;
        PrintSetWaterSchedule1();
        break;
      case 2:
        ws1TimeOnHour--;
        if(ws1TimeOnHour<0){
          ws1TimeOnHour = 23;
        }
        PrintSetWaterSchedule1();
        break;
      case 3:
        ws1TimeOnMinute--;
        if(ws1TimeOnMinute<0){
          ws1TimeOnMinute=59;
        }
        PrintSetWaterSchedule1();
        break;
      case 4:
        setWS1DurationMinute--;
        if(setWS1DurationMinute<0){
          setWS1DurationMinute=59;
        }
        ws1DurationMinute = setWS1DurationMinute;
        PrintSetWaterSchedule1();
        break;
      case 5:
        setWS1DurationSecond--;
        if(setWS1DurationSecond<0){
          setWS1DurationSecond=59;
        }
        ws1DurationSecond = setWS1DurationSecond;
        PrintSetWaterSchedule1();
        break;
      default:
        Serial.print("default - Left - SetWaterSchedule1Buttons");
    }
  }
  // RIGHT
  else if (rightState == ON) {
    Serial.println("Right - ");
    switch(setWaterSchedule1Index){
      case 1:
        waterSchedule1IsOn = true;
        PrintSetWaterSchedule1();
        break;
      case 2:
        ws1TimeOnHour++;
        if(ws1TimeOnHour>23){
          ws1TimeOnHour = 0;
        }
        PrintSetWaterSchedule1();
        break;
      case 3:
        ws1TimeOnMinute++;
        if(ws1TimeOnMinute>59){
          ws1TimeOnMinute=0;
        }
        PrintSetWaterSchedule1();
        break;
      case 4:
        setWS1DurationMinute++;
        if(setWS1DurationMinute>59){
          setWS1DurationMinute=0;
        }
        ws1DurationMinute = setWS1DurationMinute;
        PrintSetWaterSchedule1();
        break;
      case 5:
        setWS1DurationSecond++;
        if(setWS1DurationSecond>59){
          setWS1DurationSecond=0;
        }
        ws1DurationSecond = setWS1DurationSecond;
        PrintSetWaterSchedule1();
        break;
      case 6:
        EEPROM.write(hourAddress,           ws1TimeOnHour);
        EEPROM.write(minuteAddress,         ws1TimeOnMinute);
        EEPROM.write(durationMinuteAddress, setWS1DurationMinute);
        EEPROM.write(durationSecondAddress, setWS1DurationSecond);
        lcd.setCursor(0,0);
        lcd.print("Schedule Saved!");
        break;
      default:
        Serial.print("default - Right - SetWaterSchedule1Buttons");
    }
  }
  // SELECT
  else if (selectState == ON) {
    // nothing
  }
  // BACK
  else if (backState == ON) {
    setWaterSchedule1Index = 0;
    PrintMainMenu(2);
  }
  else {
    digitalWrite(ledPin, OFF);
  }
}


// Relay Logic
void Relay_OnOff(int valveRelay, int plantNum){
  if(digitalRead(valveRelay) == ON_RELAY){
    PrintWaterPlant(plantNum,"OFF");
    digitalWrite(valveRelay, OFF_RELAY);
  } 
  else if(digitalRead(valveRelay) == OFF_RELAY) {
    PrintWaterPlant(plantNum,"ON");
    digitalWrite(valveRelay, ON_RELAY);
  }  
}
void WaterSchedule1RelayControl(){
  if(!waterSchedule1IsOn){return;}

  if(rtc_hour == ws1TimeOnHour && rtc_minute == ws1TimeOnMinute && !delayTimer){
    timeToWater = true;
  }
  if(rtc_minute == ws1TimeOnMinute+1){
    delayTimer = false;
  }
  if(!timeToWater || delayTimer){return;}

  currentSecondsTimer = rtc_second;
  if(currentSecondsTimer - previousSecondsTimer >= oneSecondInterval){
    previousSecondsTimer = currentSecondsTimer;
    ws1DurationSecond--;

    if(ws1DurationMinute<=0 && ws1DurationSecond < 0){
        timeToWater       = false;
        delayTimer        = true;
        ws1DurationMinute = setWS1DurationMinute;
        ws1DurationSecond = setWS1DurationSecond;
        digitalWrite(relay, ON); //OFF
    }
    else if(ws1DurationSecond<0){
      ws1DurationMinute--;
      ws1DurationSecond = 59;
    }
    if(!delayTimer){
      digitalWrite(relay, OFF);
    }
  }
}


// Time Functions
void TheTime(){
  currentSeconds = rtc_second;
  if (currentSeconds - previousSeconds >= oneSecondInterval) {
    previousSeconds = currentSeconds;

    //Pages with time
    if(mainMenuIndex == 1 && currentMenu == mainMenu){
      PrintDate();
      PrintTime();
      PrintTemperatureHumidity();
    }
    if(setWaterSchedule1Index == 0 && currentMenu == waterSchedule1 && waterSchedule1IsOn){
      lcd.setCursor(9,1);
      if(ws1DurationMinute<10){
          lcd.print("0");
        }
      lcd.print(ws1DurationMinute);
      lcd.print("m ");
      if(ws1DurationSecond<10){
          lcd.print("0");
        }
      lcd.print(ws1DurationSecond);
      lcd.print("s");
    }
  }
}
void RTCTime(){
  DateTime now = rtc.now(); 

  rtc_year   = now.year() % 100;
  rtc_month  = now.month();
  rtc_day    = now.day();
  rtc_hour   = now.hour();
  rtc_minute = now.minute();
  rtc_second = now.second();
}


// Temperature-Humidity Functions
void TemperatureHumidity(){
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  // Check if reading was successful
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error reading from DHT sensor!");
  } else {
    // Display temperature and humidity on the serial monitor
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println("°C");
  }

  // Wait for a few seconds before taking another reading
  // delay(2000);
}


// Helper Functions
void SetWaterScheduleTopCursor(){
    lcd.setCursor(1,0);
  }
  


void setup() {
  Serial.begin(9600);
  BlinkForSetup();
  SetupPins();
  MoreSetupStuff();
  SetupLCD();
}

void loop() {
  TheTime();
  CheckButtonInput();
  MenuButtonsFunctions();
  WaterSchedule1RelayControl();
  RTCTime();
}
//****************************************************************
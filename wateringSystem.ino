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
int eeAddress = 0;


// Pins
const int back         = 2;
const int home         = 3;
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
unsigned long lastDebounceTime     = 0;
unsigned long debounceDelay        = 150;
unsigned long previousSeconds      = 0;
unsigned long currentSeconds       = 0;
unsigned long previousSecondsTimer = 0;
unsigned long currentSecondsTimer  = 0;


float humidity;
float temperature;

bool waterSchedule1IsOn;
bool wateredToday;
bool timeToWater;
bool delayTimer;

int ON_LED    = HIGH;
int OFF_LED   = LOW;
int ON_RELAY  = LOW;
int OFF_RELAY = HIGH;




// Setup
void BlinkForSetup(){
  for(int i = 3; i >= 0; i--){
    digitalWrite(ledPin, ON_LED);
    delay(250);
    digitalWrite(ledPin, OFF_LED);
    delay(50);
  }
  for(int i = 4; i >= 0; i--){
    digitalWrite(ledPin, ON_LED);
    delay(50);
    digitalWrite(ledPin, OFF_LED);
    delay(50);
  }
}
void SetupPins(){
  pinMode(back,  INPUT);
  pinMode(home,  INPUT);
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
  currentMenu        = "MainMenu";
  waterSchedule1IsOn = true;
  wateredToday       = false;
  timeToWater        = false;
  delayTimer         = false;

  dht.begin();        // Initialize DHT sensor
  rtc.begin();        // Initialize the RTC

  // Uncomment the following line to set the initial time and date (optional)
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

  // PrintWaterScheduleOnOff();
}
void PrintTemperatureHumidity(){
  humidity = dht.readHumidity();
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
void PrintWaterScheduleOnOff(){
  if(!waterSchedule1IsOn){return;}
  lcd.setCursor(0,1);
  lcd.print("1");
}
void PrintGoBack(){
  lcd.setCursor(0,0);
  lcd.print("Go Back");
  lcd.setCursor(0,1);
  lcd.print("<-");
}
void PrintWaterPlant(int plantNum, String onOffString){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Water #");
  lcd.print(plantNum);
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
      lcd.print("Go Back");
      lcd.setCursor(0,1);
      lcd.print("<-");
      break;
    
    default:
      Serial.print("default - setWaterSchedule1Index - PrintSetWaterSchedule1");
  }
}




// Button Logic
void CheckButtonInput(){
  upState = digitalRead(up);
  downState = digitalRead(down);
  leftState = digitalRead(left);
  rightState = digitalRead(right);
  backState = digitalRead(back);
  selectState = digitalRead(home);
  delay(debounceDelay);
}
void MenuButtonsFunctions(){
  MainMenuButtons();
  // SetTimeDateMenuButtons();
  SetWaterSchedule1Buttons();
}
void MainMenuButtons(){
  if(currentMenu != mainMenu){return;}
  // UP
  if (upState == HIGH) {
    digitalWrite(ledPin, HIGH);
    mainMenuIndex++;
    if(mainMenuIndex > mainMenuIndexMax){
      mainMenuIndex = 0;
    }
    PrintMainMenu(mainMenuIndex);
  } 
  // DOWN
  else if (downState == HIGH) {
    digitalWrite(ledPin, HIGH);
    mainMenuIndex--;
    if(mainMenuIndex < 0){
      mainMenuIndex = mainMenuIndexMax;
    }
    PrintMainMenu(mainMenuIndex);
  }
  // LEFT
  else if (leftState == HIGH) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Left - Main Menu");
    switch(mainMenuIndex){
      case 2:
        Relay_OnOff(relay, 1);
        break;
      case 3:
        break;
      default:
        Serial.print("default - Left - mainMenuIndex");
    }
  }
  // RIGHT
  else if (rightState == HIGH) {
    digitalWrite(ledPin, HIGH);
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
      default:
        Serial.print("default case - MainMenuButtons - Select");
    }
  }
  // BACK
  else if (backState == HIGH) {
    digitalWrite(ledPin, HIGH);
    Serial.println("Back - Main Menu");
    mainMenuIndex = 1;
    PrintMainMenu(1);
  }
  // SELECT
  else if (selectState == HIGH) {
    digitalWrite(ledPin, HIGH);
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
    digitalWrite(ledPin, LOW);
  }
}
void SetWaterSchedule1Buttons(){
  if(currentMenu != waterSchedule1){return;}
  // UP
  if(upState == HIGH) {
    setWaterSchedule1Index++;
    if(setWaterSchedule1Index > setWaterSchedule1IndexMax){
      setWaterSchedule1Index = 0;
    }
    PrintSetWaterSchedule1();
  } 
  // DOWN
  else if (downState == HIGH) {
    setWaterSchedule1Index--;
    if(setWaterSchedule1Index < 0){
      setWaterSchedule1Index = setWaterSchedule1IndexMax;
    }
    PrintSetWaterSchedule1();
  }
  // LEFT
  else if(leftState == HIGH) {
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
  else if (rightState == HIGH) {
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
        setWaterSchedule1Index = 0;
        PrintMainMenu(2);
        break;
      default:
        Serial.print("default - Right - SetWaterSchedule1Buttons");
    }
  }
  // SELECT
  else if (selectState == HIGH) {
    // nothing
  }
  // BACK
  else if (backState == HIGH) {
    setWaterSchedule1Index = 0;
    PrintMainMenu(2);
  }
  else {
    digitalWrite(ledPin, LOW);
    // digitalWrite(valveRelay, LOW);
  }
}


// Relay Logic
void Relay_OnOff(int valveRelay, int plantNum){
  if(digitalRead(valveRelay) == LOW){
    PrintWaterPlant(plantNum,"OFF");
    digitalWrite(valveRelay, HIGH);
  } 
  else if(digitalRead(valveRelay) == HIGH) {
    PrintWaterPlant(plantNum,"ON");
    digitalWrite(valveRelay, LOW);
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
        digitalWrite(relay, HIGH); //OFF
    }
    else if(ws1DurationSecond<0){
      ws1DurationMinute--;
      ws1DurationSecond = 59;
    }
    if(!delayTimer){
      digitalWrite(relay, LOW);
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

  // EEPROM.write(address, 42);

  // Serial.print("EEPROM-ADR:0 = ");
  // Serial.println(EEPROM.read(address));
}

void loop() {
  TheTime();
  CheckButtonInput();
  MenuButtonsFunctions();
  WaterSchedule1RelayControl();
  RTCTime();
}
//****************************************************************
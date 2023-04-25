#include "DFRobot_PH.h"
#include "DFRobot_EC.h"

#include <LiquidCrystal_I2C.h>
#include <LiquidMenu.h>

#include <EEPROM.h>
#include "uRTCLib.h"
#include <OneWire.h>
#include <Wire.h>
#include <SD.h> 

//*** Setup ***///

#define PH_PIN A14
#define EC_PIN A13
#define T_PIN A15
#define pumpEc 4
#define pumpPh 5
#define butMenu 3
#define butOk 2
#define butUp 18
#define butDown 19
#define pumptime 500

bool changed = 0;
int screenState = 1;
int lineState = 0;
unsigned long lastInterrupt;
float  voltagePH,voltageEC,phValue,ecValue,temperature = 25;
float  phSetpoint= 25;
float ecSetpoint = 25 ;
int pumpTime = 2000; 
unsigned long sdCurrentMillis,sdPreviousMillis;
bool sdState = LOW;

OneWire ds(T_PIN);
//****Screen and Menu Setting****//

uRTCLib RTC(0x68);

LiquidCrystal_I2C lcd(0x27, 16, 2); //Setting with Pin LCD

LiquidLine welcome_1(1, 0, "Nutri Auto 1.0");
LiquidLine welcome_2(5,1, "Welcome");
LiquidScreen welcomeScreen(welcome_1,welcome_2);
LiquidLine notiEc(0,0,"EC: ",ecValue);
LiquidLine notiPh(0,1,"PH: ",phValue);
LiquidScreen notiScreen(notiEc,notiPh);
LiquidLine setupPh(0,0,"Set pH:",phSetpoint);
LiquidLine setupEc(0,1,"Set EC:",ecSetpoint);
LiquidScreen setupScreen(setupPh,setupEc);
LiquidLine setupPhSetpoint1(0,0,"pH Setpoint:");
LiquidLine setupPhSetpoint2(0,1,phSetpoint);
LiquidScreen phsetupScreen(setupPhSetpoint1,setupPhSetpoint2);
LiquidLine setupECSetpoint1(0,0,"EC Setpoint:");
LiquidLine setupECSetpoint2(0,1,ecSetpoint);
LiquidScreen ecsetupScreen(setupECSetpoint1,setupECSetpoint2);
LiquidMenu menu(lcd,notiScreen,setupScreen,phsetupScreen,ecsetupScreen);

//*** Button Setting ***//



DFRobot_PH ph;
DFRobot_EC ec;

void screen_Update(){
  switch (screenState){
    case 1: 
      if(changed){
        menu.change_screen(&notiScreen);
        changed = 0;
      }
      break;
    case 2:
      if(changed){
        menu.change_screen(&setupScreen);
        changed = 0;
      }
      break;
    case 3:
      if(changed){
        menu.change_screen(&phsetupScreen);
        changed = 0;
      }
      break;
    case 4:
      if(changed){
        menu.change_screen(&ecsetupScreen);
        changed = 0;
      }
      break;
    case 5:
      if(changed){
        menu.switch_focus(false);
        menu.update();
        lineState = menu.get_focusedLine();
        changed = 0;
      }
      break;
      case 6:
      if(changed){
        menu.switch_focus(true);
        menu.update();
        lineState = menu.get_focusedLine();
        changed = 0;
      }
      break;
      case 7: 
        if(changed){
          phSetpoint = phSetpoint+ 0.1;
          menu.update();
          changed = 0;
        }
      break;
      case 8: 
        if(changed){
          phSetpoint = phSetpoint- 0.1;
          menu.update();
          changed = 0;
        }
      break;
      case 9: 
        if(changed){
          ecSetpoint = ecSetpoint+ 0.1;
          menu.update();
          changed = 0;
        }
      break;
      case 10: 
        if(changed){
          ecSetpoint = ecSetpoint- 0.1;
          menu.update();
          changed = 0;
        }
      break;
  
  }
  Serial.println(phSetpoint);
}

void interruptMenu(){
  if(millis() - lastInterrupt > 30) // we set a 10ms no-interrupts window
    {    
    lastInterrupt = millis();
    Serial.println("okkk");
    screenState = 2;
    changed = 1;
    }
  
 

}
void interruptOk(){
  if(millis() - lastInterrupt > 30){ 
    if(screenState == 5 || screenState == 6){
      if (screenState == 5){
        screenState = 3;
        changed = 1;
      }
      if(screenState == 6){
        screenState = 4;
        changed = 1;
      }
    }
    if(screenState == 7 || screenState == 8 ||screenState == 9 ||screenState == 10 ){
      screenState = 1;
      changed =1;
    }
  }  
}
void interruptUp(){
  if(millis() - lastInterrupt > 30){ 
    if(screenState == 2|| screenState == 5 || screenState == 6){
      screenState = 5;
      changed = 1;
    }
  
    if(screenState == 3||screenState == 7||screenState == 8){
      screenState == 7;
      changed =1;
    }
    if(screenState == 4||screenState == 9||screenState == 10){
      screenState = 9;
      changed =1;
    }
  }
}
void interruptDown(){
  if(millis() - lastInterrupt > 30){ 
    if(screenState == 2 || screenState == 5 || screenState == 6){
      screenState = 6;
      changed =1;
    }
    if(screenState == 3||screenState == 7||screenState == 8){
      screenState = 8;
      changed = 1;
    }
    if(screenState == 4 ||screenState == 9||screenState == 10){
      screenState = 10;
      changed =1;
    }
  }
}

void setup()
{
    Serial.begin(115200);  
    ph.begin();
    ec.begin();
    SDSetup();
    pinMode(4,OUTPUT);  //2
    pinMode(5,OUTPUT);  //3
    pinMode(6,OUTPUT);  //4
    pinMode(7,OUTPUT);  //1
    lcd.init();
    lcd.backlight();
    pinMode(butMenu, INPUT);
    pinMode(butOk,INPUT);
    pinMode(butUp,INPUT);
    pinMode(butDown,INPUT);
    attachInterrupt(digitalPinToInterrupt(butMenu),interruptMenu, RISING);
    attachInterrupt(digitalPinToInterrupt(butOk),interruptOk, RISING);
    attachInterrupt(digitalPinToInterrupt(butUp),interruptUp, RISING);
    attachInterrupt(digitalPinToInterrupt(butDown),interruptDown, RISING);
    menu.init();
    menu.add_screen(welcomeScreen);
    menu.add_screen(notiScreen);
    menu.add_screen(setupScreen);
    menu.add_screen(phsetupScreen);
    menu.add_screen(ecsetupScreen);
    menu.change_screen(&welcomeScreen);
    delay(1000);
    menu.change_screen(&notiScreen);
    menu.update();
    digitalWrite(7,HIGH);
    digitalWrite(4,HIGH);
    digitalWrite(5,HIGH);
    digitalWrite(6,HIGH);
}

void loop()
{   
  screen_Update();
  readEc();
  readPh();
  controlEc();
  controlPh();
  SDLoop();
}
//************* Pump Control******************//

void controlEc(){
 readEc();
 while (ecValue < (ecSetpoint*0.97)){
   readEc();
   digitalWrite(pumpEc,);LOW
   delay(pumpTime);
   digitalWrite(pumpEc,HIGH);
 }
}
void controlPh(){
 readPh();
 while (phValue< (phSetpoint*0.97)){
    readPh();
     digitalWrite(pumpPh,LOW);
     delay(pumptime);
     digitalWrite(pumpPh,HIGH);
 }

}

//************* Read Sensor ******************//

float readEc(){
static unsigned long timepoint = millis();
    if(millis()-timepoint>1000U)  //time interval: 1s
    {
      timepoint = millis();
      voltageEC = analogRead(EC_PIN)/1024.0*5000;  // read the voltage

      readTemperature();  // read your temperature sensor to execute temperature compensation
      ecValue =  ec.readEC(voltageEC,temperature);  // convert voltage to EC with temperature compensation
      Serial.print("  temperature:");
      Serial.print(temperature,1);
      Serial.print("^C  EC:");
      Serial.print(ecValue,1);
      Serial.println("ms/cm");
    }
    ec.calibration(voltageEC,temperature);
}

void readPh(){
  voltagePH = analogRead(PH_PIN)/1024.0*5000;          // read the ph voltage
  phValue    = ph.readPH(voltagePH,temperature);       // convert voltage to pH with temperature compensation
  Serial.print("pH:");
  Serial.print(phValue,2);
}

float readTemperature()
{
  //add your code here to get the temperature from your temperature sensor
  float temperature = getTemp();
}

float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad


  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return TemperatureSum;

}
//************** SD Card ***************//

void SDSetup()
{
    pinMode(53, OUTPUT);

    if (!SD.begin(53))
    {
        return;
    }
}

void SDLoop()
{
    unsigned long sdCurrentMillis = millis();
    if (sdCurrentMillis - sdPreviousMillis > 3000)
    {
        sdPreviousMillis = sdCurrentMillis;
        if (sdState == LOW)
        {
            sdState = HIGH;
            File dataFile = SD.open("datalog.csv", FILE_WRITE);

            if (dataFile)
            {
//                DateTime now = RTC.now();
                dataFile.print(RTC.day(), DEC);
                dataFile.print('/');
                dataFile.print(RTC.month(), DEC);
                dataFile.print('/');
                dataFile.print(RTC.year(), DEC);
                dataFile.print(' ');
                dataFile.print(RTC.hour(), DEC);
                dataFile.print(':');
                dataFile.print(RTC.minute(), DEC);
                dataFile.print(':');
                dataFile.print(RTC.second(), DEC);
                dataFile.print(", ");
                dataFile.print(phValue);
                dataFile.print(", ");
                dataFile.print(ecValue);
                dataFile.print(", ");
                dataFile.print(temperature);
                dataFile.println();
                dataFile.close();
            }
        }
        else
        {
            sdState = LOW;
        }
    }
}

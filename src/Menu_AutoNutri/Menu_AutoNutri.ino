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
#define EC_PIN A15
#define T_PIN 10
#define pumpEc 4
#define pumpPh 8 
#define butMenu 18 
#define butOk 3
#define butUp 2
#define butDown 19
#define pumptime 3000

bool isFirst = true;
float ecArray[20];
float phArray[20];
bool changed = 0;
int screenState = 0;
int lineState = 0;
unsigned long lastInterrupt;
float  voltagePH,voltageEC,phValue,ecValue,temperature = 0.0;
static unsigned long timepoint;
float  phSetpoint= 7.0;
float ecSetpoint = 0.5 ;
int pumpTime = 2000; 
unsigned long sdCurrentMillis,sdPreviousMillis;
bool sdState = LOW;
bool stateRead = 0;
OneWire ds(T_PIN);
//****Screen and Menu Setting****//
//
uRTCLib RTC(0x68);

DFRobot_PH ph;
DFRobot_EC ec;
float average (float * array, float newValue)  // assuming array is int.
{
  float sum = 0.0 ;  // sum will be larger than an item, long for safety.
  for (int i = 0; i < 9 ;i++){
    array[i] = array[i+1];
  }
  array[9] = newValue;
  for (int i = 0 ; i < 10 ; i++){
    sum += array[i] ;
  }
  return   sum/10.0;  // average will be fractional, so float may be appropriate.
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
float readTemperature()
{
  //add your code here to get the temperature from your temperature sensor
  float temperature = getTemp();
  return temperature;
}   

float readEcph(){
    
    if(millis()-timepoint > 10000)  //time interval: 1s
    { 
      if (stateRead == 0){
        digitalWrite(7,LOW);
        digitalWrite(6,LOW);
        delay(100);
        int i = 0;
        while(i<10){
        voltageEC = analogRead(EC_PIN)/1024.0*5000;  // read the voltage
        temperature = readTemperature();  // read your temperature sensor to execute temperature compensation
        if(ec.readEC(voltageEC,temperature)>=0.05){
          ecValue = average(ecArray,ec.readEC(voltageEC,temperature))+0.23;  
        }
        else{
          ecValue = average(ecArray,ec.readEC(voltageEC,temperature));
        }
        
        Serial.print(ec.readEC(voltageEC,temperature));
        Serial.print(" ");
        ec.calibration(voltageEC,temperature);
        i++;
        delay(100);
        }  
        digitalWrite(7,HIGH);
        digitalWrite(6,HIGH);
        timepoint = millis();
        stateRead = 1;
      }
      else {
        int j = 0;
        while(j< 10){
        voltagePH = analogRead(PH_PIN)/1024.0*5000;
        temperature = readTemperature();
        ph.readPH(voltagePH,temperature); // read the ph voltage
        
        phValue    = average(phArray,ph.readPH(voltagePH,temperature))+0.25;
        
        Serial.print(phValue);
        Serial.println(" ");  
    // convert voltage to pH with temperature compensation
        ph.calibration(voltagePH,temperature);
        j++;
        delay(100);
      }
        timepoint = millis();
        stateRead = 0;
        }
    }
}





LiquidCrystal_I2C lcd(0x27, 16, 2); //Setting with Pin LCD

LiquidLine welcome_1(1, 0, "Nutri Auto 1.0");
LiquidLine welcome_2(5,1, "Welcome");
LiquidScreen welcomeScreen(welcome_1,welcome_2);
LiquidLine notiEc(0,0,"EC: ",ecValue);
LiquidLine notiPh(0,1,"PH: ",phValue);
LiquidLine notiT(10,0,getTemp);
LiquidScreen notiScreen(notiEc,notiPh,notiT);
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
          phSetpoint = phSetpoint + 0.1;
          menu.update();
          changed = 0;
        }
      break;
      case 8: 
        if(changed){
          phSetpoint = phSetpoint - 0.1;
          menu.update();
          changed = 0;
        }
      break;
      case 9: 
        if(changed){
          ecSetpoint = ecSetpoint+ 0.05;
          menu.update();
          changed = 0;
        }
      break;
      case 10: 
        if(changed){
          ecSetpoint = ecSetpoint- 0.05;
          menu.update();
          changed = 0;
        }
      break;
  
  }
}

void interruptMenu(){
  if(millis() - lastInterrupt > 30 && isFirst) // we set a 10ms no-interrupts window
    {   
      isFirst = false; 
    }
    else 
    {
      lastInterrupt = millis();
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
    pinMode(4,OUTPUT);  //EC1
    pinMode(8,OUTPUT);  //EC2
    pinMode(6,OUTPUT);  //PH
    pinMode(7,OUTPUT);  //EC
    pinMode(9,INPUT);
    lcd.init();
    lcd.backlight();
    pinMode(butMenu, INPUT);
    pinMode(butOk,INPUT);
    pinMode(butUp,INPUT);
    pinMode(butDown,INPUT);
    digitalWrite(7,HIGH);
    digitalWrite(4,HIGH);
    digitalWrite(8,HIGH);
    digitalWrite(6,HIGH);
    menu.init();
    menu.add_screen(welcomeScreen);
    menu.add_screen(notiScreen);
    menu.add_screen(setupScreen);
    menu.add_screen(phsetupScreen);
    menu.add_screen(ecsetupScreen);
    menu.change_screen(&welcomeScreen);
    menu.change_screen(&notiScreen);
    menu.update();
    attachInterrupt(digitalPinToInterrupt(butMenu),interruptMenu, RISING);
    attachInterrupt(digitalPinToInterrupt(butOk),interruptOk, RISING);
    attachInterrupt(digitalPinToInterrupt(butUp),interruptUp, RISING);
    attachInterrupt(digitalPinToInterrupt(butDown),interruptDown, RISING);
    

    delay(1000);

    int i = 0;
    for (i = 0; i < 20; i++)
    {
      ecArray[i] = 0.0;
      phArray[i] = 0.0;
    }

    timepoint = millis(); 
}

void loop()
{   
  readEcph();
  screen_Update();
  menu.update();
  
  controlEc();
//  SDLoop();
  delay(100);
}
//************* Pump Control******************//

void controlEc(){
Serial.print(ecValue);
Serial.print(" ");
  Serial.print(ecSetpoint);
  Serial.println(" ");
 if (ecValue <= (ecSetpoint)&& ecValue >=0.25){
    digitalWrite(pumpEc,LOW);
 }
 else{
   
   digitalWrite(pumpEc,HIGH);
   controlPh();
 }
}
void controlPh(){
  Serial.print(phValue);
  Serial.print(" ");
  Serial.print(phSetpoint);
  Serial.println(" ");
 if (phValue >= (phSetpoint)){
     digitalWrite(pumpPh,LOW);  
     Serial.println("?");
      
 }
 else {
     digitalWrite(pumpPh,HIGH);
     Serial.println("????");
 }


}

//************* Read Sensor ******************//


//************** SD Card ***************//
//
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

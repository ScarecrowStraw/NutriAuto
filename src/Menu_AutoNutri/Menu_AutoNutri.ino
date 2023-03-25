#include "DFRobot_PH.h"
#include "DFRobot_EC.h"

#include <LiquidCrystal_I2C.h>
#include <LiquidMenu.h>

#include <EEPROM.h>
#include <RTClib.h> 
#include <Wire.h>
#include <SD.h> 

//****Screen and Menu Setting****//

LiquidCrystal_I2C lcd(0x3F, 16, 2);; //Setting with Pin LCD
LiquidLine welcome_1(1, 0, "Nutri Auto");
LiquidLine welcome_2(0,1, "Alo 123 456");
LiquidScreen welcomeScreen(welcome_1,welcome_2);
LiquidLine notiEc(0,0,"EC: ",ecValue);
LiquidLine notiPh(0,1,"PH: ",phValue);
LiquidScreen notiScreen(notiEc,notiPh);
LiquidMenu menu(lcd);

//*** Button Setting ***//

Button left(A0, pullup);
Button right(7, pullup);
Button up(8, pullup);
Button down(9, pullup);
Button enter(10, pullup);

//*** Sensor Setup ***///
#define PH_PIN A1
#define EC_PIN A2

float  voltagePH,voltageEC,phValue,ecValue,temperature = 25;

DFRobot_PH ph;
DFRobot_EC ec;



void setup()
{
    Serial.begin(115200);  
    ph.begin();
    ec.begin();
    SDSetup(); 
    lcd.init();
    lcd.backlight();
    menu.init();
    menu.add_screen(welcomeScreen);
    menu.add_screen(notiScreen);
}

void loop()
{
  readEcph();
  SDLoop();
  menu.update();
}

//************* Read Sensor ******************//

void readEcph(){
  voltagePH = analogRead(PH_PIN)/1024.0*5000;          // read the ph voltage
  phValue    = ph.readPH(voltagePH,temperature);       // convert voltage to pH with temperature compensation
  Serial.print("pH:");
  Serial.print(phValue,2);
  voltageEC = analogRead(EC_PIN)/1024.0*5000;
  ecValue    = ec.readEC(voltageEC,temperature);       // convert voltage to EC with temperature compensation
  Serial.print(", EC:");
  Serial.print(ecValue,2);
  Serial.println("ms/cm");
}
float readTemperature()
{
  //add your code here to get the temperature from your temperature sensor
}

//************** SD Card ***************//

void SDSetup()
{
    pinMode(53, OUTPUT);

    if (!SD.begin(chipSelect))
    {
        return;
    }
}

void SDLoop()
{
    unsigned long sdCurrentMillis = millis();
    if (sdCurrentMillis - sdPreviousMillis > sdTime)
    {
        sdPreviousMillis = sdCurrentMillis;
        if (sdState == LOW)
        {
            sdState = HIGH;
            File dataFile = SD.open("datalog.csv", FILE_WRITE);

            if (dataFile)
            {
                now = RTC.now();
                dataFile.print(now.day(), DEC);
                dataFile.print('/');
                dataFile.print(now.month(), DEC);
                dataFile.print('/');
                dataFile.print(now.year(), DEC);
                dataFile.print(' ');
                dataFile.print(now.hour(), DEC);
                dataFile.print(':');
                dataFile.print(now.minute(), DEC);
                dataFile.print(':');
                dataFile.print(now.second(), DEC);
                dataFile.print(", ");
                dataFile.print(pH);
                dataFile.print(", ");
                dataFile.print(pmem);
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
//******* Button Check ***********//

void buttonsCheck() {
  if (right.check() == LOW) {
    menu_system.next_screen();
  }
  if (left.check() == LOW) {
    menu_system.previous_screen();
  }
  if (up.check() == LOW) {
    menu_system.call_function(increase);
  }
  if (down.check() == LOW) {
    menu_system.call_function(decrease);
  }
  if (enter.check() == LOW) {
    menu_system.switch_focus();
  }
}

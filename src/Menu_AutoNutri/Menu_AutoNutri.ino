#include "DFRobot_PH.h"
#include "DFRobot_EC.h"

#include <LiquidCrystal_I2C.h>
#include <LiquidMenu.h>

#include <EEPROM.h>
//#include <RTClib.h> 
#include <Wire.h>
#include <SD.h> 

//*** Setup ***///

#define PH_PIN A15
#define EC_PIN A14
#define T_PIN A13
#define pumpEc 3
#define pumpPh 4
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

<<<<<<< HEAD
float readEc();
float readPh();
=======
>>>>>>> 73b9c0c ([Update] update menu setting + pump control)

//****Screen and Menu Setting****//

LiquidCrystal_I2C lcd(0x27, 16, 2); //Setting with Pin LCD

LiquidLine welcome_1(1, 0, "Nutri Auto 1.0");
LiquidLine welcome_2(5,1, "Welcome");
LiquidScreen welcomeScreen(welcome_1,welcome_2);
LiquidLine notiEc(0,0,"EC: ",ecSetpoint);
LiquidLine notiPh(0,1,"PH: ",phSetpoint);
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
//    ph.begin();
//    ec.begin();
//    SDSetup();
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
}

void loop()
{   
  screen_Update();
  //readEcph();
  //SDLoop();
}
//************* Pump Control******************//

//void controlEc(){
//  while (readEc()< (ecSetpoint*0.97)){
//    digitalWrite(pumpEc,HIGH);
//    delay(pumpTime);
//    digitalWrite(pumpEc,LOW);
//  }
//}
//void controlPh(){
//  while (readPh()< (phSetpoint*0.97)){
//      digitalWrite(pumpPh,HIGH);
//      delay(pumptime);
//      digitalWrite(pumpPh,LOW);
//  }
//
//}

//************* Read Sensor ******************//

//float readEc(){
//  voltageEC = analogRead(EC_PIN)/1024.0*5000;
//  ecValue    = ec.readEC(voltageEC,temperature);       // convert voltage to EC with temperature compensation
//  for(int i=0;i<9;i++){
//      for(int j=i+1;j<10;j++){
//        if(buffer_arr[i]>buffer_arr[j]){
//          temp=buffer_arr[i];
//          buffer_arr[i]=buffer_arr[j];
//          buffer_arr[j]=temp;
//        }
//     }
// }
//  for(int i=2;i<8;i++)
//    avgval+=buffer_arr[i];
//    float volt=(float)avgval*5.0/1024/6;
//    float ph_act = -5.70 * volt + calibration_value;
//}
//  return ph_act;
//}
//
//float readPh(){
//  voltagePH = analogRead(PH_PIN)/1024.0*5000;          // read the ph voltage
//  phValue    = ph.readPH(voltagePH,temperature);       // convert voltage to pH with temperature compensation
//  Serial.print("pH:");
//  Serial.print(phValue,2);
//  return phValue;
//}

float readTemperature()
{
  //add your code here to get the temperature from your temperature sensor
}

//************** SD Card ***************//

// void SDSetup()
// {
//     pinMode(53, OUTPUT);

//     if (!SD.begin(chipSelect))
//     {
//         return;
//     }
// }

// void SDLoop()
// {
//     unsigned long sdCurrentMillis = millis();
//     if (sdCurrentMillis - sdPreviousMillis > sdTime)
//     {
//         sdPreviousMillis = sdCurrentMillis;
//         if (sdState == LOW)
//         {
//             sdState = HIGH;
//             File dataFile = SD.open("datalog.csv", FILE_WRITE);

//             if (dataFile)
//             {
//                 now = RTC.now();
//                 dataFile.print(now.day(), DEC);
//                 dataFile.print('/');
//                 dataFile.print(now.month(), DEC);
//                 dataFile.print('/');
//                 dataFile.print(now.year(), DEC);
//                 dataFile.print(' ');
//                 dataFile.print(now.hour(), DEC);
//                 dataFile.print(':');
//                 dataFile.print(now.minute(), DEC);
//                 dataFile.print(':');
//                 dataFile.print(now.second(), DEC);
//                 dataFile.print(", ");
//                 dataFile.print(pH);
//                 dataFile.print(", ");
//                 dataFile.print(pmem);
//                 dataFile.println();
//                 dataFile.close();
//             }
//         }
//         else
//         {
//             sdState = LOW;
//         }
//     }
// }
//******* Button Check ***********//

// void buttonsCheck() {
//   if (right.check() == LOW) {
//     menu_system.next_screen();
//   }
//   if (left.check() == LOW) {
//     menu_system.previous_screen();
//   }
//   if (up.check() == LOW) {
//     menu_system.call_function(increase);
//   }
//   if (down.check() == LOW) {
//     menu_system.call_function(decrease);
//   }
//   if (enter.check() == LOW) {
//     menu_system.switch_focus();
//   }
// }

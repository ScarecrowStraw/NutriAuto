#include <SD.h>       //SD card library
#include <Wire.h>     //One Wire library

#include <RTClib.h> //Real Time Clock library

int pHPin = A2;       // pin for pH probe

const int chipSelect = 53; // pin for chipselect SD card

RTC_DS1307 RTC; // Define RTC module

//*********Declaring Variables************************************//

float pH;                   // generates the value of pH
float Offset = 0.00;        // deviation from true pH compensation (if necessary)
unsigned long int avgValue; // stores the average value of the pH sensor
int buf[14], temp;

const int numReadings = 10;
int readings[numReadings]; // the readings from the analog input
int index = 0;             // the index of the current reading
int total = 0;             // the running total
int average = 0;           // the average

int count = 0;



int sdState = LOW;         // variables for delayed writing to SD card
long sdPreviousMillis = 0; //             |
long sdTime = 7500;        //             |

float Setpoint;     // holds value for Setpoint
float HysterisMin;  // Minimum deviation from Setpoint
float HysterisPlus; // Maximum deviation from Setpoint
float SetHysteris;  // Holds the value for Hysteris

DateTime now; // call current Date and Time

// Every block is in a separate function.
void setup() 
{
    smoothArraySetup(); // Sets the array for smoothing the pH value to 0
    logicSetup();       // Replaces the void Setup
    timeSetup();        // Initialises the RTC module
    SDSetup();          // Initialises the SD module
    seconds_elapsed_total = 0;
}

void loop()
{
    logicLoop(); // pH algorithm loops through this one, also the smooting of the signal
    SDLoop();          // Writing all sensor data to SD
}

// LCD setup
void LCDInit() // initialises the lcd screen
{
    // LCD setting
}




void logicSetup()
{

    pinMode(pHPlusPin, OUTPUT);
    pinMode(pHMinPin, OUTPUT);
    pinMode(solenoidPin, OUTPUT);
    delay(300);
}


void logicLoop(){
    
}

//*********RTC Functions*********//

void timeSetup()
{
    Wire.begin();
    RTC.begin();
}

//*********SDCard Functions*********//

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

//*********Button Functions*********//

void phIncreaseSetpoint()              //Increase Setpoint   
{
    Setpoint = Setpoint + 0.01;
    if (Setpoint >= 9.00)
    {
        Setpoint = 9.00;
    }
}


void phDecreaseSetpoint()               //Decrease Setpoint  
{
    Setpoint = Setpoint - 0.01;
    if (Setpoint <= 3.00)
    {
        Setpoint = 3.00;
    }
}

void phIncreaseHysteris()
{
    SetHysteris = SetHysteris + 0.01;
    if (SetHysteris >= 9.00)
    {
        SetHysteris = 9.00;
    }
}


void phDecreaseHysteris()
{
    SetHysteris = SetHysteris - 0.01;
    if (SetHysteris <= 0.01)
    {
        SetHysteris = 0.01;
    }
}


void DecreasePumpHighTime()
{
    pinHighTime = pinHighTime - 10;
    if (pinHighTime <= 0)
    {
        pinHighTime = 0;
    }
}


void IncreasePumpHighTime()
{
    pinHighTime = pinHighTime + 10;
    if (pinHighTime >= 2000)
    {
        pinHighTime = 2000;
    }
}

void DecreasePumpLowTime()
{
    pinLowTime = pinLowTime - 100;
    if (pinLowTime <= 0)
    {
        pinLowTime = 0;
    }
}


void IncreasePumpLowTime()
{
    pinLowTime = pinLowTime + 100;
    if (pinLowTime >= 20000)
    {
        pinLowTime = 20000;
    }
}

//*********Control Functions*********//


//*********Sensor Functions*********//

void smoothArraySetup()
{
    for (int thisReading = 0; thisReading < numReadings; thisReading++)
    {
        readings[thisReading] = 0;
    }
}
double avergearray(int* arr, int number){
  int i;
  int max,min;
  double avg;
  long amount=0;
  if(number<=0){
    Serial.println("Error number for the array to avraging!/n");
    return 0;
  }
  if(number<5){   //less than 5, calculated directly statistics
    for(i=0;i<number;i++){
      amount+=arr[i];
    }
    avg = amount/number;
    return avg;
  }else{
    if(arr[0]<arr[1]){
      min = arr[0];max=arr[1];
    }
    else{
      min=arr[1];max=arr[0];
    }
    for(i=2;i<number;i++){
      if(arr[i]<min){
        amount+=min;        //arr<min
        min=arr[i];
      }else {
        if(arr[i]>max){
          amount+=max;    //arr>max
          max=arr[i];
        }else{
          amount+=arr[i]; //min<=arr<=max
        }
      }//if
    }//for
    avg = (double)amount/(number-2);
  }//if
  return avg;
}
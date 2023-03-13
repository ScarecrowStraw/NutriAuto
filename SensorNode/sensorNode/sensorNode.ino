#include <SD.h>       //SD card library
#include <Wire.h>     //One Wire library

#include <RTClib.h> //Real Time Clock library

int pHPin = A2;       // pin for pH probe
int pHPlusPin = 45;   // pin for Base pump (relay)
int pHMinPin = 43;    // pin for Acid pump (relay)
int floatLowPin = 8;  // pin for lower float sensor
int floatHighPin = 7; // pin for upper float sensor
int solenoidPin = 47; // pin for Solenoid valve (relay)

const int chipSelect = 53; // pin for chipselect SD card

RTC_DS1307 RTC; // Define RTC module

//*********Declaring Variables************************************//
int tankProgState = 1;      // returns the state of tank program - on or off
float pH;                   // generates the value of pH
boolean smoothPh = 0;       // variable that sets smoothing of pH on or off
float Offset = 0.00;        // deviation from true pH compensation (if necessary)
unsigned long int avgValue; // stores the average value of the pH sensor
float b;
int buf[14], temp;

const int numReadings = 10;
int readings[numReadings]; // the readings from the analog input
int index = 0;             // the index of the current reading
int total = 0;             // the running total
int average = 0;           // the average

int count = 0;

int ledState = LOW;      // variables for pulsing the pump
long previousMillis = 0; //             |
long pinHighTime = 100;  //             |
long pinLowTime = 7500;  //             |
long pinTime = 100;      //             |

int sdState = LOW;         // variables for delayed writing to SD card
long sdPreviousMillis = 0; //             |
long sdTime = 7500;        //             |

int pmem = 0;       // check which page your on
float Setpoint;     // holds value for Setpoint
float HysterisMin;  // Minimum deviation from Setpoint
float HysterisPlus; // Maximum deviation from Setpoint
float SetHysteris;  // Holds the value for Hysteris

float FanTemp;         // Holds the set value for temperature
float FanHumid;        // Holds the set value for humidity
float fanHysteris = 2; // Set value for hysteris tuning Fan
float LightTime;       // Holds the set value for amount of time plants should have light

int lightADCReading;      // variables for measuring the light
double currentLightInLux; //              |
double lightInputVoltage; //              |
double lightResistance;   //              |

float LightOn;                              // Time the plants have had light
float proportion_to_light = LightTime / 24; // calculate desired hours of light total and supplemental daily based on above values
float seconds_light = 0;
float proportion_lit;
float seconds_elapsed;
float seconds_elapsed_total;
float seconds_for_this_cycle;

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

    TankProgControl(); // Conrolling loop for refilling the tank

    SDLoop();          // Writing all sensor data to SD
}

// LCD setup
void LCDInit() // initialises the lcd screen
{
    // LCD setting
}

void smoothArraySetup()
{
    for (int thisReading = 0; thisReading < numReadings; thisReading++)
    {
        readings[thisReading] = 0;
    }
}


void logicSetup()
{

    pinMode(pHPlusPin, OUTPUT);
    pinMode(pHMinPin, OUTPUT);
    pinMode(solenoidPin, OUTPUT);

    pmem == 0;

    delay(300);
}


void logicLoop()
{
    if (smoothPh == 0)                                  // If smoothPh = 0 then no smooting is used
    {                                                   //                  |
        float sensorValue = 0;                          // Default the Value = 0
        sensorValue = analogRead(pHPin);                // sensorValue gets the value from the pH probe
        pH = (sensorValue * 5.0 / 1024 * 3.5 + Offset); // pH = the calculated value
                                                        //                   |
        HysterisMin = (Setpoint - SetHysteris);  // HysterisMin = the lowest value that is allowed by the program. Lower then this and a Base is added.
        HysterisPlus = (Setpoint + SetHysteris); // HysterisPlus = the highest value that is allowed by the program. Higher and an acid is added.
                                                 //                   |
        if (pmem == 0)                                   // If pmem equals 0, then goto the next if statement.
        {                                                //                  |
            if (pH < HysterisMin)                        // If pH is smaller then HysterisMin, then set pmem to 1
            {                                            //                  |
                pmem = 1;                                //                  |
            }                                            //                  |
                                                         //                  |
            if (pH >= HysterisMin && pH <= HysterisPlus) // If pH is greater or the same as HysterisMin AND pH is smaller of the same as HysterisPlus, then do nothing.
            {                                            //                  |
                digitalWrite(pHPlusPin, LOW);            // Set base pump to off position
                digitalWrite(pHMinPin, LOW);             // Set acid pump to off position
            }                                            //                  |
                                                         //                  |
            if (pH > HysterisPlus)                       // If pH is greater then HysterisPlus, set pmem to 2
            {                                            //                  |
                pmem = 2;                                //                  |
            }                                            //                  |
        }                                                //                  |
                                                         //                  |
                                                         //                  |
        if (pmem == 1)                                   // If pmem equals 1, the goto next if statement
        {                                                //                  |
            if (pH < HysterisMin)                        // If pH is smaller then HysterisMin, then
            {
                unsigned long currentMillis = millis();
                if (currentMillis - previousMillis > pinTime)
                {
                    previousMillis = currentMillis;

                    if (ledState == LOW)
                    {
                        ledState = HIGH;
                        pinTime = pinHighTime;
                    }
                    else
                    {
                        ledState = LOW;
                        pinTime = pinLowTime;
                    }
                    digitalWrite(pHPlusPin, ledState);
                    digitalWrite(pHMinPin, LOW);
                }
            }

            if (pH >= HysterisMin && pH < Setpoint)
            {
                unsigned long currentMillis = millis();
                if (currentMillis - previousMillis > pinTime)
                {
                    previousMillis = currentMillis;

                    if (ledState == LOW)
                    {
                        ledState = HIGH;
                        pinTime = pinHighTime;
                    }
                    else
                    {
                        ledState = LOW;
                        pinTime = pinLowTime;
                    }
                    digitalWrite(pHPlusPin, ledState);
                    digitalWrite(pHMinPin, LOW);
                }
            }

            if (pH >= Setpoint)
            {
                pmem = 0;
            }
        }

        if (pmem == 2)
        {
            if (pH > HysterisPlus)
            {
                unsigned long currentMillis = millis();
                if (currentMillis - previousMillis > pinTime)
                {
                    previousMillis = currentMillis;

                    if (ledState == LOW)
                    {
                        ledState = HIGH;
                        pinTime = pinHighTime;
                    }
                    else
                    {
                        ledState = LOW;
                        pinTime = pinLowTime;
                    }
                    digitalWrite(pHMinPin, ledState);
                    digitalWrite(pHPlusPin, LOW);
                }
            }

            if (pH <= HysterisPlus && pH > Setpoint)
            {
                unsigned long currentMillis = millis();
                if (currentMillis - previousMillis > pinTime)
                {
                    previousMillis = currentMillis;

                    if (ledState == LOW)
                    {
                        ledState = HIGH;
                        pinTime = pinHighTime;
                    }
                    else
                    {
                        ledState = LOW;
                        pinTime = pinLowTime;
                    }
                    digitalWrite(pHMinPin, ledState);
                    digitalWrite(pHPlusPin, LOW);
                }
            }

            if (pH <= Setpoint)
            {
                pmem = 0;
            }
        }
    }
    if (smoothPh == 1)
    {
        // total = total - readings[index];
        // readings[index] = analogRead(pHPin);
        // total = total + readings[index];
        // index = index + 1;

        // if (index >= numReadings)
        // {
        //  index = 0;
        // }

        //  average = total / numReadings;

        // float sensorValue = 0;
        // sensorValue = average;
        // pH = (0.0178 * sensorValue - 1.889);

        for (int i = 0; i < 14; i++)
        {
            buf[i] = analogRead(pHPin);
            delay(10);
        }
        for (int i = 0; i < 13; i++) // sort the analog values from small to large
        {
            for (int j = i + 1; j < 14; j++)
            {
                if (buf[i] > buf[j])
                {
                    temp = buf[i];
                    buf[i] = buf[j];
                    buf[j] = temp;
                }
            }
        }
        avgValue = 0;
        for (int i = 2; i < 10; i++) // take the average value of 10 center sample
            avgValue += buf[i];
        float phValue = (float)avgValue * 5.0 / 1024 / 10; // convert the analog into millivolt
        pH = 3.5 * phValue;                                // convert the millivolt into pH

        HysterisMin = (Setpoint - SetHysteris);
        HysterisPlus = (Setpoint + SetHysteris);

        ++count;
        if (count > 10)
        {
            count = 10;
        }

        if (count == 10)
        {
            if (pmem == 0)
            {
                if (pH < HysterisMin)
                {
                    pmem = 1;
                }

                if (pH >= HysterisMin && pH <= HysterisPlus)
                {
                    digitalWrite(pHPlusPin, LOW);
                    digitalWrite(pHMinPin, LOW);
                }

                if (pH > HysterisPlus)
                {
                    pmem = 2;
                }
            }

            if (pmem == 1)
            {
                if (pH < HysterisMin)
                {
                    unsigned long currentMillis = millis();
                    if (currentMillis - previousMillis > pinTime)
                    {
                        previousMillis = currentMillis;

                        if (ledState == LOW)
                        {
                            ledState = HIGH;
                            pinTime = pinHighTime;
                        }
                        else
                        {
                            ledState = LOW;
                            pinTime = pinLowTime;
                        }
                        digitalWrite(pHPlusPin, ledState);
                        digitalWrite(pHMinPin, LOW);
                    }
                }

                if (pH >= HysterisMin && pH < Setpoint)
                {
                    unsigned long currentMillis = millis();
                    if (currentMillis - previousMillis > pinTime)
                    {
                        previousMillis = currentMillis;

                        if (ledState == LOW)
                        {
                            ledState = HIGH;
                            pinTime = pinHighTime;
                        }
                        else
                        {
                            ledState = LOW;
                            pinTime = pinLowTime;
                        }
                        digitalWrite(pHPlusPin, ledState);
                        digitalWrite(pHMinPin, LOW);
                    }
                }

                if (pH >= Setpoint)
                {
                    pmem = 0;
                }
            }

            if (pmem == 2)
            {
                if (pH > HysterisPlus)
                {
                    unsigned long currentMillis = millis();
                    if (currentMillis - previousMillis > pinTime)
                    {
                        previousMillis = currentMillis;

                        if (ledState == LOW)
                        {
                            ledState = HIGH;
                            pinTime = pinHighTime;
                        }
                        else
                        {
                            ledState = LOW;
                            pinTime = pinLowTime;
                        }
                        digitalWrite(pHMinPin, ledState);
                        digitalWrite(pHPlusPin, LOW);
                    }
                }

                if (pH <= HysterisPlus && pH > Setpoint)
                {
                    unsigned long currentMillis = millis();
                    if (currentMillis - previousMillis > pinTime)
                    {
                        previousMillis = currentMillis;

                        if (ledState == LOW)
                        {
                            ledState = HIGH;
                            pinTime = pinHighTime;
                        }
                        else
                        {
                            ledState = LOW;
                            pinTime = pinLowTime;
                        }
                        digitalWrite(pHMinPin, ledState);
                        digitalWrite(pHPlusPin, LOW);
                    }
                }

                if (pH <= Setpoint)
                {
                    pmem = 0;
                }
            }
        }
    }
    delay(250);
}

void phIncreaseSetpoint()
{
    Setpoint = Setpoint + 0.01;
    if (Setpoint >= 9.00)
    {
        Setpoint = 9.00;
    }
}


void phDecreaseSetpoint()
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


void TankProgControl()
{
    if (tankProgState == 0)
    {
    }
    if (tankProgState == 1)
    {
        int levelHigh = LOW;
        int levelLow = LOW;

        levelHigh = digitalRead(floatHighPin);
        levelLow = digitalRead(floatLowPin);

        if (levelHigh == LOW)
        {
            if (levelLow == LOW)
            {
                digitalWrite(solenoidPin, HIGH); // solenoid valve open.
            }
        }
        else
        {
            if (levelLow == HIGH)
            {
                digitalWrite(solenoidPin, LOW); // solenoid valve closed.
            }
        }
    }
}

void ManualRefilProg()
{
    digitalWrite(solenoidPin, HIGH);
}

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

void timeSetup()
{
    Wire.begin();
    RTC.begin();
}

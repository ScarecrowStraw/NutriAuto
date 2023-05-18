/*
    ------ Waspmote Pro Code Example --------

    Explanation: This is the basic Code for Waspmote Pro

    Copyright (C) 2016 Libelium Comunicaciones Distribuidas S.L.
    http://www.libelium.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <WaspWIFI_PRO_V3.h>
#include <WaspSensorXtr.h>

// choose socket (SELECT USER'S SOCKET)
///////////////////////////////////////
uint8_t socket = SOCKET0;
///////////////////////////////////////

// WiFi AP settings (CHANGE TO USER'S AP)
///////////////////////////////////////
char SSID[] = "K_CNNN";
char PASSW[] = "congnghenn";
//char SSID[] = "Mai Phuc Lam";
//char PASSW[] = "05100912";

//char SSID[] = "AVITECH_IDS2";
//char PASSW[] = "avitechuet";
///////////////////////////////////////

//Variables use to connect to AP
uint8_t error;
uint8_t status;
unsigned long previous;

//MQTT server, port, username and password to public to topic
//char MQTT_SERVER[] = "513booyoungct4.ddns.net";
//uint16_t MQTT_PORT = 1883;
//char username[] = "admin";
//char password[] = "admin13589";

//MQTT server, port, username and password to public to topic (MQTT broker on embedded computer)
char MQTT_SERVER[] = "192.168.1.13";
uint16_t MQTT_PORT = 1883;
char username[]= "libe";
char password[]= "123456";

//Variable to use with weather station
weatherStation mySensor;
uint8_t response = 0;

void setup()
{
  USB.ON();
  USB.println(F("Start program"));

 //////////////////////////////////////////////////
  // 1. Switch ON weather station to read
  //////////////////////////////////////////////////
  
  mySensor.ON();
  
  //Recommended waiting time since power on
  delay(10000);

  //////////////////////////////////////////////////
  // 2. Configure Wifi conection
  //////////////////////////////////////////////////
  configure_Wifi();
  
  //////////////////////////////////////////////////
  // 3. Configure HTTP conection
  //////////////////////////////////////////////////

  //error = WIFI_PRO_V3.mqttConfiguration(HTTP_SERVER,"user", HTTP_PORT, WaspWIFI_v3::MQTT_TLS_DISABLED);
  error = WIFI_PRO_V3.mqttConfiguration(MQTT_SERVER, "user", MQTT_PORT, 0, username, password);
  if (error == 0)
  {
    USB.println(F("3. MQTT conection configured"));
  }
  else
  {
    USB.print(F("3. MQTT conection configured ERROR"));
  }
  
}

void loop()
{
  // Check if Wifi is connected or not
  while (WIFI_PRO_V3.isConnected() == false)
  {
    configure_Wifi();
  }
  //////////////////////////////////////////////////
  // 4. Read data from weather station
  //////////////////////////////////////////////////
  response = mySensor.read();
  while (response == 0)
  {
    //USB.println(response);
    USB.println(F("Sensor not connected or invalid data"));
    mySensor.OFF();
    delay(3000);
    mySensor.ON();
  //  delay(5000);
    delay(10000);
    response = mySensor.read();
  }

  //USB.println(response);
  USB.println(F("Sensor connected "));
  printWeatherData_to_SerialMonitor();
  
  //Parameters needed in Direction project
  uint16_t solarRadiation = mySensor.gmx.solarRadiation; // (W/m2)
  float rainFall = mySensor.gmx.precipTotal; // mySensor.gmx.precipIntensity (mm)
  uint16_t relativeHumidity = mySensor.gmx.relativeHumidity; //measured relativeHumidity in %
  float temperature = mySensor.gmx.temperature; //(degree celsius)
  float windSpeed = mySensor.gmx.windSpeed; //mySensor.gmx.avgWindSpeed (m/s)
//  char* timestamp;
//  timestamp = mySensor.gmx.timestamp;
  
   
  //delay(5000);

  //////////////////////////////////////////////////
  // 5. Public data to MQTT broker
  //////////////////////////////////////////////////

//  uint16_t solarRadiation = 15000; // (W/m2)
//  float rainFall = 1200.55; // mySensor.gmx.precipIntensity (mm)
//  uint16_t relativeHumidity = 10000; //measured relativeHumidity in %
//  float temperature = 28.75; //(degree celsius)
//  float windSpeed = 1121.123; //mySensor.gmx.avgWindSpeed (m/s)
  //char timestamp[22] = "19-03-2023T17:00:00";
  
  
  //Prepare message to public to MQTT broker
  char message[100];
  char solarRadiation_string[10];
  char rainFall_string[10];
  char relativeHumidity_string[10];
  char temperature_string[10];
  char windSpeed_string[10];
 
  dtostrf(solarRadiation, 1, 0, solarRadiation_string);
  dtostrf(rainFall, 4, 2, rainFall_string);
  dtostrf(relativeHumidity, 1, 0, relativeHumidity_string);
  dtostrf(temperature, 4, 2, temperature_string);
  dtostrf(windSpeed, 4, 2, windSpeed_string);
  //rad 50;rai 12.550;hu 100;tem 28.750;win 2.123

  sprintf(message, "rad: %s;rai: %s;h: %s;t: %s;w: %s", solarRadiation_string, rainFall_string, relativeHumidity_string, temperature_string, windSpeed_string);
  //sprintf(message, "radi %s;rain %s;humi %s;temp %s;wind %s", solarRadiation_string, rainFall_string, relativeHumidity_string, temperature_string, windSpeed_string);
  //sprintf(message, "radiation %s;rainFall %s;relativeHumidity %s", solarRadiation_string, rainFall_string, relativeHumidity_string);
  USB.println(message);
  
 //Public message to MQTT broker
  error = WIFI_PRO_V3.mqttPublishTopic("/sensor/weatherStation",WaspWIFI_v3::QOS_2,WaspWIFI_v3::RETAINED, message);
  if (error == 0)
  {
    USB.println(F("Publish message done!"));
  }
  else
  {
    USB.println(F("Error publishing message to topic!"));
  }

 delay(30000);
 //delay(1000);
 
}

void printWeatherData_to_SerialMonitor()
{
  USB.println(F("---------------------------"));
    USB.println(F("GMX"));

    USB.print(F("Wind direction: "));
    USB.print(mySensor.gmx.windDirection);
    USB.println(F(" degrees"));

    USB.print(F("Avg. wind dir: "));
    USB.print(mySensor.gmx.avgWindDirection);
    USB.println(F(" degrees"));

    USB.print(F("Cor. wind dir: "));
    USB.print(mySensor.gmx.correctedWindDirection);
    USB.println(F(" degrees"));

    USB.print(F("Avg. cor. wind dir:"));
    USB.print(mySensor.gmx.avgCorrectedWindDirection);
    USB.println(F(" degrees"));

    USB.print(F("Avg. wind gust dir: "));
    USB.print(mySensor.gmx.avgWindGustDirection);
    USB.println(F(" degrees"));

    USB.print(F("Wind speed: "));
    USB.printFloat(mySensor.gmx.windSpeed, 2);
    USB.println(F(" m/s"));

    USB.print(F("Avg. wind speed: "));
    USB.printFloat(mySensor.gmx.avgWindSpeed, 2);
    USB.println(F(" m/s"));

    USB.print(F("Avg. wind gust speed:"));
    USB.printFloat(mySensor.gmx.avgWindGustSpeed, 2);
    USB.println(F(" m/s"));

    USB.print(F("Wind sensor status: "));
    USB.println(mySensor.gmx.windSensorStatus);

    USB.print(F("Precip. total: "));
    USB.printFloat(mySensor.gmx.precipTotal, 3);
    USB.println(F(" mm"));

    USB.print(F("Precip. int: "));
    USB.printFloat(mySensor.gmx.precipIntensity, 3);
    USB.println(F(" mm"));

    USB.print(F("Precip. status: "));
    USB.println(mySensor.gmx.precipStatus, DEC);
    
    USB.print(F("Solar radiation: "));
    USB.print(mySensor.gmx.solarRadiation);
    USB.println(F(" W/m^2"));

    USB.print(F("Sunshine hours: "));
    USB.printFloat(mySensor.gmx.sunshineHours, 2);
    USB.println(F(" hours"));

    USB.print(F("Sunrise: "));
    USB.print(mySensor.gmx.sunriseTime);
    USB.println(F(" (h:min)"));

    USB.print(F("Solar noon: "));
    USB.print(mySensor.gmx.solarNoonTime);
    USB.println(F(" (h:min)"));

    USB.print(F("Sunset: "));
    USB.print(mySensor.gmx.sunsetTime);
    USB.println(F(" (h:min)"));

    USB.print(F("Sun position: "));
    USB.print(mySensor.gmx.sunPosition);
    USB.println(F(" (degrees:degrees)"));

    USB.print(F("Twilight civil: "));
    USB.print(mySensor.gmx.twilightCivil);
    USB.println(F(" (h:min)"));

    USB.print(F("Twilight nautical: "));
    USB.print(mySensor.gmx.twilightNautical);
    USB.println(F(" (h:min)"));

    USB.print(F("Twilight astronomical: "));
    USB.print(mySensor.gmx.twilightAstronom);
    USB.println(F(" (h:min)"));
    
    USB.print(F("Barometric pressure: "));
    USB.printFloat(mySensor.gmx.pressure, 1);
    USB.println(F(" hPa"));

    USB.print(F("Pressure at sea level: "));
    USB.printFloat(mySensor.gmx.pressureSeaLevel, 1);
    USB.println(F(" hPa"));

    USB.print(F("Pressure at station: "));
    USB.printFloat(mySensor.gmx.pressureStation, 1);
    USB.println(F(" hPa"));

    USB.print(F("Relative humidity: "));
    USB.print(mySensor.gmx.relativeHumidity);
    USB.println(F(" %"));

    USB.print(F("Air temperature: "));
    USB.printFloat(mySensor.gmx.temperature, 1);
    USB.println(F(" Celsius degrees"));

    USB.print(F("Dew point: "));
    USB.printFloat(mySensor.gmx.dewpoint, 1);
    USB.println(F(" degrees"));

    USB.print(F("Absolute humidity: "));
    USB.printFloat(mySensor.gmx.absoluteHumidity, 2);
    USB.println(F(" g/m^3"));

    USB.print(F("Air density: "));
    USB.printFloat(mySensor.gmx.airDensity, 1);
    USB.println(F(" Kg/m^3"));

    USB.print(F("Wet bulb temperature: "));
    USB.printFloat(mySensor.gmx.wetBulbTemperature, 1);
    USB.println(F(" Celsius degrees"));

    USB.print(F("Wind chill: "));
    USB.printFloat(mySensor.gmx.windChill,1);
    USB.println(F(" Celsius degrees"));
    
    USB.print(F("Heat index: "));
    USB.print(mySensor.gmx.heatIndex);
    USB.println(F(" Celsius degrees"));
    
    USB.print(F("Compass: "));
    USB.print(mySensor.gmx.compass);
    USB.println(F(" degrees"));

    USB.print(F("X tilt: "));
    USB.printFloat(mySensor.gmx.xTilt, 0);
    USB.println(F(" degrees"));

    USB.print(F("Y tilt: "));
    USB.printFloat(mySensor.gmx.yTilt, 0);
    USB.println(F(" degrees"));

    USB.print(F("Z orient: "));
    USB.printFloat(mySensor.gmx.zOrient, 0);
    USB.println();

    USB.print(F("Timestamp: "));
    USB.println(mySensor.gmx.timestamp);

    USB.print(F("Voltage: "));
    USB.printFloat(mySensor.gmx.supplyVoltage, 1);
    USB.println(F(" V"));

    USB.print(F("Status: "));
    USB.println(mySensor.gmx.status);
    
    USB.println(F("---------------------------\n"));
  
} 

void configure_Wifi()
{
  //////////////////////////////////////////////////
  // 1. Switch ON the WiFi module
  //////////////////////////////////////////////////
  error = WIFI_PRO_V3.ON(socket);

  if (error == 0)
  {
    USB.println(F("1. WiFi switched ON"));
  }
  else
  {
    USB.println(F("1. WiFi did not initialize correctly"));
  }

  //////////////////////////////////////////////////
  // 2. Reset to default values
  //////////////////////////////////////////////////
  error = WIFI_PRO_V3.resetValues();

  if (error == 0)
  {
    USB.println(F("2. WiFi reset to default"));
  }
  else
  {
    USB.println(F("2. WiFi reset to default ERROR"));
  }

  ////////////////////////////////////////////////
  // 3. Configure mode (Station or AP)
  //////////////////////////////////////////////////
  error = WIFI_PRO_V3.configureMode(WaspWIFI_v3::MODE_STATION);

  if (error == 0)
  {
    USB.println(F("3. WiFi configured OK"));
  }
  else
  {
    USB.println(F("3. WiFi configured ERROR"));
  }

  //////////////////////////////////////////////////
  // 4. Configure SSID and password
  //////////////////////////////////////////////////
  error = WIFI_PRO_V3.configureStation(SSID, PASSW);

  if (error == 0)
  {
    USB.println(F("4. WiFi configured SSID OK"));
  }
  else
  {
    USB.println(F("4. WiFi configured SSID ERROR"));
  }

  // get current time
  previous = millis();

  //////////////////////////////////////////////////
  // 5. Connect to AP
  //////////////////////////////////////////////////
  error = WIFI_PRO_V3.connect();

  if (error == 0)
  {
    USB.println(F("5. WiFi connected to AP OK"));

    USB.print(F("SSID: "));
    USB.println(WIFI_PRO_V3._essid);
    
    USB.print(F("Channel: "));
    USB.println(WIFI_PRO_V3._channel, DEC);

    USB.print(F("Signal strength: "));
    USB.print(WIFI_PRO_V3._power, DEC);
    USB.println("dB");

    USB.print(F("IP address: "));
    USB.println(WIFI_PRO_V3._ip);

    USB.print(F("GW address: "));
    USB.println(WIFI_PRO_V3._gw);

    USB.print(F("Netmask address: "));
    USB.println(WIFI_PRO_V3._netmask);

    WIFI_PRO_V3.getMAC();

    USB.print(F("MAC address: "));
    USB.println(WIFI_PRO_V3._mac);
  }
  else
  {
    USB.println(F("5. WiFi connect to AP ERROR"));

    USB.print(F("Disconnect status: "));
    USB.println(WIFI_PRO_V3._status, DEC);

    USB.print(F("Disconnect reason: "));
    USB.println(WIFI_PRO_V3._reason, DEC);
  }

}


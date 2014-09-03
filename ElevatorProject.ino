#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <Process.h>
#include <Bridge.h>
#include <Temboo.h>
#include "TembooAccount.h" // contains Temboo account information, as described below



/*
 Elevator Project
 created 18 Aug 2014
 by Tom Igoe

 Uses Arduino Bridge library and Adafruit BMP085 library.
 Uses dweet.io as well.

*/
//

Adafruit_BMP085 bmp;
int lastFloor = 0;
int delayInterval = 1000;
int maxDelay = 2;
int updateCount = 0;   // Execution count, so this doesn't run forever
String command;

float currentAtmospherePressure = 0;

void setup() {


  Serial.begin(9600);
  Serial.println("Starting...");
  Bridge.begin();
  Serial.println("Bridge started...");
  
  //FIXME:do this on yun first time setup.
  Process p;
  p.runShellCommand("date --set=\"2014-09-02 20:55:00\"");
  while (p.running());
  // Read command output. runShellCommand() should have passed "Signal: xx&":
  while (p.available() > 0) {
    char c = p.read();
    Serial.print(c);
  }

  bmp.begin();
  Serial.println(readSignal());
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  delay(100);
  digitalWrite(8, HIGH);
  command.reserve(200);

}

void loop() {
   
  if(updateCount > maxDelay ){
    getLatestWeather();
    updateCount = 0;
  }
  readPressure();
  delay(delayInterval);
  updateCount++;
  
}

void readPressure(){
    command = "curl -k \"https://dweet.io/dweet/for/tisch-elevator1?altitude=";

  Process curl;
  Serial.print("Temperature = ");
  Serial.print(bmp.readTemperature());
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(bmp.readPressure());
  Serial.println(" Pa");

  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  Serial.print("Altitude = ");
  Serial.print(bmp.readAltitude());
  Serial.println(" meters");

  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.
  Serial.print("Real altitude = ");
  float myAltitude = bmp.readAltitude(101000);
  Serial.print(myAltitude);
  Serial.println(" meters");
  int alt = bmp.readAltitude();
  int thisFloor = map(alt, 10, 59, 1, 12);
  Serial.print("floor: ");
  Serial.println(thisFloor);

  command += alt;

  int signal = readSignal();
  command += "&rssi=";
  command += signal;
  command += "&floor=";
  command += thisFloor;

  command += "\"";
  Serial.println();
  Serial.println(command);
  if (thisFloor != lastFloor) {
    curl.runShellCommand(command);
    while (curl.running());
    while (curl.available() > 0) {
      char c = curl.read();
      Serial.print(c);
    }
    Serial.println();
    lastFloor = thisFloor;
  }
}

int readSignal() {
  Process wifiCheck;  // initialize a new process
  int signalStrength = 0;
  wifiCheck.runShellCommand("/usr/bin/pretty-wifi-info.lua");  // command you want to run

  // while there's any characters coming back from the
  // process, print them to the serial monitor:
  while (wifiCheck.available() > 0) {
    if (wifiCheck.find("Signal")) {
      signalStrength = wifiCheck.parseInt();
    }
  }
  return signalStrength;
}

void getLatestWeather(){
    TembooChoreo GetWeatherByAddressChoreo;

    // Invoke the Temboo client
    GetWeatherByAddressChoreo.begin();
    
    // Set Temboo account credentials
    GetWeatherByAddressChoreo.setAccountName(TEMBOO_ACCOUNT);
    GetWeatherByAddressChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
    GetWeatherByAddressChoreo.setAppKey(TEMBOO_APP_KEY);
    
    // Set Choreo inputs
    //using c to get pressure in millibars
    GetWeatherByAddressChoreo.addInput("Units", "c");
//        GetWeatherByAddressChoreo.addInput("Units", "f");
    GetWeatherByAddressChoreo.addInput("Address", "721 Broadway, NY 10003");
    
    // Identify the Choreo to run
    GetWeatherByAddressChoreo.setChoreo("/Library/Yahoo/Weather/GetWeatherByAddress");
    GetWeatherByAddressChoreo.addOutputFilter("pressure", "/rss/channel/yweather:atmosphere/@pressure", "Response");

    // Run the Choreo; when results are available, print them to serial
    GetWeatherByAddressChoreo.run();
    String responseString ="";
    while(GetWeatherByAddressChoreo.available()) {
        responseString += (char)GetWeatherByAddressChoreo.read();
//      1 millibar =100 pascals
    }
//    Serial.print(responseString);
   
    String result = getValue(responseString,'\n',1);
    result.trim();
    Serial.print("CURRENT PRESSURE FROM YAHOO : ");
    Serial.print(result);
    Serial.println(" millibars");
//    float f = 10; 
//    f = atof(result.c_str());
    GetWeatherByAddressChoreo.close();

}

 String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}

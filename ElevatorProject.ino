/*

 Elevator Project
 created 18 Aug 2014
 by Tom Igoe

 Uses Arduino Bridge library and Adafruit BMP085 library.
 Uses dweet.io as well.

*/

#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <Process.h>
#include <Bridge.h>

Adafruit_BMP085 bmp;
int lastFloor = 0;
int delayInterval = 1000;
int maxDelay = 2;
int updateCount = 0;   // Execution count, so this doesn't run forever
String command;

float currentAtmospherePressure = 101000;
float minAltitude = 25;
float maxAltitude = 75;

void setup() {


  Serial.begin(9600);
  Serial.println("Starting...");
  Bridge.begin();
  Serial.println("Bridge started...");

  //FIXME:do this on yun first time setup.
  Process p;
  p.runShellCommand("date --set=\"2014-08-07 17:37:00\"");
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

  if (updateCount > maxDelay ) {
    curlLatestWeather();
    updateCount = 0;
  }
  readPressure();
  //  curlLatestWeather();
  delay(delayInterval);
  updateCount++;

}

void readPressure() {

  //  Serial.print("Temperature = ");
  //  Serial.print(bmp.readTemperature());
  //  Serial.println(" *C");
  //
  //  Serial.print("Pressure = ");
  //  Serial.print(bmp.readPressure());
  //  Serial.println(" Pa");

  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  //  Serial.print("Altitude = ");
  //  Serial.print(bmp.readAltitude());
  //  Serial.println(" meters");
  //  Serial.print("Sea Level Pressure = ");
  //  Serial.println(bmp.readSealevelPressure());
  ////  Serial.println(" meters");

  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.

  Serial.print("Current Atmosphere Pressure = ");
  Serial.println(currentAtmospherePressure);
  float myAltitude = bmp.readAltitude(currentAtmospherePressure);

  Serial.print("Real altitude = ");
  Serial.print(myAltitude);
  Serial.println(" meters");

  // Recalibrate min and max
  minAltitude = (myAltitude < minAltitude) ? myAltitude : minAltitude;
  maxAltitude = (myAltitude > maxAltitude) ? myAltitude : maxAltitude;

  int thisFloor = map(myAltitude, minAltitude, maxAltitude, 1, 12);
  Serial.print("floor: ");
  Serial.println(thisFloor);

  Process curl;
  command = "curl -k \"https://dweet.io/dweet/for/tisch-elevator1?altitude=";
  command += myAltitude;

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

void curlLatestWeather() {
  Process curl;
  String weatherCommand =  "curl \"http://weather.yahooapis.com/forecastrss?w=12761335&u=c\" | egrep -o pressure=\\\"[0-9]*\\.?[0-9]*\\\" | egrep -o \\\"[0-9]*\\.\\?[0-9]*\\\"";
  curl.runShellCommand(weatherCommand);
  while (curl.running());
  String result;
  while (curl.available() > 0) {
    char c = curl.read();
    if (c != '\"') {
      result += c;
    }
  }
  Serial.print(result);
  float x = result.toFloat();
  //1 millibar =100 pascals
  currentAtmospherePressure = x * 100;
  Serial.print(" Updated currentAtmospherePressure : ");
  Serial.println(currentAtmospherePressure);
  Serial.println();
}

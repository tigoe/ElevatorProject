#include <Wire.h>
#include <BMP085.h>
#include <Process.h>

/*
 Elevator Project
 created 18 Aug 2014
 by Tom Igoe
 
 Uses Arduino Bridge library and Adafruit BMP085 library.
 Uses dweet.io as well.

*/

BMP085 bmp;
int lastFloor = 0;
String command;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting...");
  Bridge.begin();
  Serial.println("Bridge started...");

  bmp.begin();
  Serial.println(readSignal());
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  delay(100);
  digitalWrite(8, HIGH);
  command.reserve(200);

}

void loop() {
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
  delay(500);
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

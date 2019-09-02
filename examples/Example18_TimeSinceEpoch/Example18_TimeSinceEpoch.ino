/*
  By: Nathan Seidle, Adapted from Example11 by Felix Jirka
  SparkFun Electronics
  Date: July 2nd, 2019
  License: MIT. See license file for more information but you can
  basically do whatever you want with this code.

  This example shows how to configure the library for serial port use with a single wire connection using the assumeAutoPVT method.
  Saving your pins for other stuff :-)

  https://gis.stackexchange.com/questions/281223/what-is-a-gps-epoch
  Good Ellipsoid vs Geoid: http://www.geokniga.org/bookfiles/geokniga-seeber-g-satellite-geodesy-2003.pdf

  The Global Positioning System (GPS) uses its own particular time scale 'GPS time'.
  It differs from UTC by a nearly integer number of seconds. Both time scales had
  identical epochs on January 5, 1980. Because GPS time is not incremented by leap
  seconds the difference between UTC and GPS time is increasing. The unit of GPS
  time is the SI second.

  https://www.andrews.edu/~tzs/timeconv/timedisplay.php
  https://tycho.usno.navy.mil/gpstt.html

  Feel like supporting open source hardware?
  Buy a board from SparkFun!
  ZED-F9P RTK2: https://www.sparkfun.com/products/15136
  NEO-M8P RTK: https://www.sparkfun.com/products/15005
  SAM-M8Q: https://www.sparkfun.com/products/15106

  Preconditions:
  U-Blox module is configured to send cyclical PVT message
  Hardware Connections:
  Connect the U-Blox serial TX pin to Rx of Serial2 (default: GPIO16) on your ESP32
  Open the serial monitor at 115200 baud to see the output
*/

#include "SparkFun_Ublox_Arduino_Library.h" //http://librarymanager/All#SparkFun_Ublox_GPS
SFE_UBLOX_GPS myGPS;

long lastTime = 0; //Simple local timer. Limits amount if I2C traffic to Ublox module.
byte leapSeconds = 16; //Currently. See https://tycho.usno.navy.mil/gpstt.html

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ; //Wait for user to open terminal
  Serial.println("SparkFun Ublox Example");

  Wire.begin();

  if (myGPS.begin() == false) //Connect to the Ublox module using Wire port
  {
    Serial.println(F("Ublox GPS not detected at default I2C address. Please check wiring. Freezing."));
    while (1)
      ;
  }

  myGPS.setI2COutput(COM_TYPE_UBX); //Set the I2C port to output UBX only (turn off NMEA noise)
  myGPS.saveConfiguration();        //Save the current settings to flash and BBR
}

void loop()
{
  //Query module only every second. Doing it more often will just cause I2C traffic.
  //The module only responds when a new position is available
  if (millis() - lastTime > 1000)
  {
    lastTime = millis(); //Update the timer

    long latitude = myGPS.getLatitude();
    Serial.print(F("Lat: "));
    Serial.print(latitude);

    long longitude = myGPS.getLongitude();
    Serial.print(F(" Long: "));
    Serial.print(longitude);
    Serial.print(F(" (degrees * 10^-7)"));

    long altitude = myGPS.getAltitude();
    Serial.print(F(" Alt: "));
    Serial.print(altitude);
    Serial.print(F(" (mm)"));

    byte SIV = myGPS.getSIV();
    Serial.print(F(" SIV: "));
    Serial.print(SIV);

    Serial.println();

    unsigned long startTime = millis();
    unsigned long gpsTime = 0;

    int days = rdn(myGPS.getYear(), myGPS.getMonth(), myGPS.getDay()) - rdn(1980, 1, 6);
    gpsTime = days * 24L * 60 * 60;
    gpsTime += myGPS.getHour() * 60L * 60;
    gpsTime += myGPS.getMinute() * 60L;
    gpsTime += myGPS.getSecond();
    gpsTime += leapSeconds;
    unsigned long stopTime = millis();

    Serial.print("gpsTime: ");
    Serial.println(gpsTime);
    Serial.print("delta: ");
    Serial.println(stopTime - startTime);

    Serial.println();
  }
}

//Source: https://stackoverflow.com/questions/14218894/number-of-days-between-two-dates-c
int rdn(int y, int m, int d) { /* Rata Die day one is 0001-01-01 */
  if (m < 3)
    y--, m += 12;
  return 365 * y + y / 4 - y / 100 + y / 400 + (153 * m - 457) / 5 + d - 306;
}

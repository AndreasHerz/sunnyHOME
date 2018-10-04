
//#include <TinyGPS++.h>
/*
   This sample sketch should be the first you try out when you are testing a TinyGPS++
   (TinyGPSPlus) installation.  In normal use, you feed TinyGPS++ objects characters from
   a serial NMEA GPS device, but this example uses static strings for simplicity.
*/
/*
// A sample NMEA stream.
const char *gpsStream =
  "$GPRMC,045103.000,A,3014.1984,N,09749.2872,W,0.67,161.46,030913,,,A*7C\r\n"
  "$GPGGA,045104.000,3014.1985,N,09749.2873,W,1,09,1.2,211.6,M,-22.5,M,,0000*62\r\n"
  "$GPRMC,045200.000,A,3014.3820,N,09748.9514,W,36.88,65.02,030913,,,A*77\r\n"
  "$GPGGA,045201.000,3014.3864,N,09748.9411,W,1,10,1.2,200.8,M,-22.5,M,,0000*6C\r\n"
  "$GPRMC,045251.000,A,3014.4275,N,09749.0626,W,0.51,217.94,030913,,,A*7D\r\n"
  "$GPGGA,045252.000,3014.4273,N,09749.0628,W,1,09,1.3,206.9,M,-22.5,M,,0000*6F\r\n";

//Funktionen
void displayInfo();

// The TinyGPS++ object
TinyGPSPlus gps;

void setup()
{
  Serial.begin(115200);

  Serial.println(F("BasicExample.ino"));
  Serial.println(F("Basic demonstration of TinyGPS++ (no device needed)"));
  Serial.print(F("Testing TinyGPS++ library v. ")); Serial.println(TinyGPSPlus::libraryVersion());
  Serial.println(F("by Mikal Hart"));
  Serial.println();

  while (*gpsStream)
    if (gps.encode(*gpsStream++))
      displayInfo();

  Serial.println();
  Serial.println(F("Done."));
}

void loop()
{
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(F("."));
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.print(gps.time.centisecond());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
*/

/*
UBLOX_example.ino
Brian R Taylor
brian.taylor@bolderflight.com
Copyright (c) 2016 Bolder Flight Systems
Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "UBLOX.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "ESP8266WiFi.h"

// a uBlox object, which is on hardware
// serial port 1 with a baud rate of 115200
UBLOX gps(Serial1,9600);




////////////  OLED Config  //////////////
Adafruit_SSD1306 display(0);
//Error Case
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif




////////////  Funktionen  /////////////
void testdrawchar();





void setup() {
    //  Set Up //
    Serial.begin(9600);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);

    //  Display Demo  //
    // draw the first ~12 characters in the font
    display.clearDisplay();
    testdrawchar();
    display.display();
    delay(2000);
    display.clearDisplay();



  // serial to display data
  Serial.begin(115200);

  // starting communication with the GPS receiver
  gps.begin();
}

void loop() {
  // checking to see if a good packet has
  // been received and displaying some
  // of the packet data
  if(gps.readSensor()) {
    Serial.print(gps.getYear());                ///< [year], Year (UTC)
    Serial.print("\t");
    Serial.print(gps.getMonth());               ///< [month], Month, range 1..12 (UTC)
    Serial.print("\t");
    Serial.print(gps.getDay());                 ///< [day], Day of month, range 1..31 (UTC)
    Serial.print("\t");
    Serial.print(gps.getHour());                ///< [hour], Hour of day, range 0..23 (UTC)
    Serial.print("\t");
    Serial.print(gps.getMin());                 ///< [min], Minute of hour, range 0..59 (UTC)
    Serial.print("\t");
    Serial.print(gps.getSec());                 ///< [s], Seconds of minute, range 0..60 (UTC)
    Serial.print("\t");
    Serial.print(gps.getNumSatellites());       ///< [ND], Number of satellites used in Nav Solution
    Serial.print("\t");
    Serial.print(gps.getLatitude_deg(),10);     ///< [deg], Latitude
    Serial.print("\t");
    Serial.print(gps.getLongitude_deg(),10);    ///< [deg], Longitude
    Serial.print("\t");
    Serial.println(gps.getMSLHeight_ft());      ///< [ft], Height above mean sea level
  }
}


///////////  Funktionen  ///////////////


void testdrawchar(void) {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  for (uint8_t i=0; i < 168; i++) {
    if (i == '\n') continue;
    display.write(i);
    if ((i > 0) && (i % 21 == 0))
      display.println();
  }
  display.display();
  delay(1);
}

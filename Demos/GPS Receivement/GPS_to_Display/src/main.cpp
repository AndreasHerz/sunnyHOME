#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "UBLOX.h"

// a uBlox object, which is on hardware
// serial port 1 with a baud rate of 115200
UBLOX gps(Serial1, 9600);




////////////  OLED Config  //////////////
Adafruit_SSD1306 display(0);
//Error Case
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif



////////////  Funktionen  /////////////
void testdrawchar();




void setup()   {
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


    // starting communication with the GPS receiver
    gps.begin();
}


void loop() {
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(10,0);
    display.println("HELLO!!!");
    display.display();
    delay(1000);
    display.clearDisplay();
    // checking to see if a good packet has
    // been received and displaying some
    // of the packet data
    if(gps.readSensor()) {
        display.printf("GPS works!!!\n");
        display.display();
        delay(1000);
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

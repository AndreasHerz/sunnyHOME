#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "UBLOX.h"

static volatile uint16_t intTriggeredCount=0; // this variable will be changed in the ISR, and Read in main loop
// static: says this variable is only visible to function in this file, its value will persist, it is a global variable
// volatile: tells the compiler that this variables value must not be stored in a CPU register, it must exist
//   in memory at all times.  This means that every time the value of intTriggeredCount must be read or
//   changed, it has be read from memory, updated, stored back into RAM, that way, when the ISR
//   is triggered, the current value is in RAM.  Otherwise, the compiler will find ways to increase efficiency
//   of access.  One of these methods is to store it in a CPU register, but if it does that,(keeps the current
//   value in a register, when the interrupt triggers, the Interrupt access the 'old' value stored in RAM,
//   changes it, then returns to whatever part of the program it interrupted.  Because the foreground task,
//   (the one that was interrupted) has no idea the RAM value has changed, it uses the value it 'know' is
//   correct (the one in the register).

void IRAM_ATTR isr(){  //IRAM_ATTR tells the complier, that this code Must always be in the
// ESP32's IRAM, the limited 128k IRAM.  use it sparingly.
    intTriggeredCount++;
}

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

    pinMode(5,INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(5),isr,FALLING);

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
    // magic happens her
    static uint16_t old_Value=0; // this variable is only visible inside loop(),
    // but it is persistent, It is only init'ed
    // once, and each time through loop() it remembers its prior value

    if(old_Value != intTriggeredCount){
      old_Value = intTriggeredCount; // something to compare against
      Serial.printf(" Someone grounded pin 5 again, it is the %d's time! Better call the Cops!",intTriggeredCount);


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

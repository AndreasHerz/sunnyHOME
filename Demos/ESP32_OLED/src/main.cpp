#include <Arduino.h>
/*****************************************************
 * ESP32 DHT Reading and OLED diplay
 * DHT Input: ==> GPIO23.
 * 128 X 32 SSD1306  - Library by Daniel Eichhorn
 *
 * MJRoBot.org 12Sept17
 *****************************************************/

/* modified font created at http://oleddisplay.squix.ch/ */
//#include "modified_font.h"

/*  SSD1306 */
#include <SSD1306.h>
#define SDA_PIN 13// GPIO21 -> SDA
#define SCL_PIN 14// GPIO22 -> SCL
#define SSD_ADDRESS 0x3c
SSD1306  display(SSD_ADDRESS, SDA_PIN, SCL_PIN);


void setup()
{
  Serial.begin(9600);
  Serial.println("");
  Serial.println("ESP32 DHT Temperature and Humidity - OLED Display");
  Serial.println("");

  display.init();

  //display.flipScreenVertically();
  //display.setTextAlignment(TEXT_ALIGN_LEFT);
  //display.setFont(Open_Sans_Condensed_Light_20); // set a font
}

void loop()
{
  displayData();
  delay(2000);
}

/***************************************************
* Display Data
****************************************************/
void displayData()
{
  Serial.print("Temp: ==> ");
  //Serial.print(localTemp);
  Serial.print("  Hum ==> ");
  //Serial.println(localHum);

  display.clear();   // clear the display

  display.drawString(0, 0,  "HELLO");
  display.drawString(90, 0,  "Andi");

  display.display();   // write the buffer to the display
  delay(10);
}

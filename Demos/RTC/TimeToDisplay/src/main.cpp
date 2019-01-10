#include <Arduino.h>
#include <Wire.h>
#include "OLED/Adafruit_SSD1306.h"
#include <time.h>
#include "TinyGPS++.h"
#include <HardwareSerial.h>



Adafruit_SSD1306 display(2/*x3c, 21, 22*/);

///   GPS Globals
HardwareSerial SerialGPS(2);    //UART0=0, UART1=1, and UART2 =2
#define RXD2 17
#define TXD2 16
// The TinyGPS++ object
TinyGPSPlus gps;
///   GPS Functions
static void smartDelay(unsigned long ms);
static void printFloat(float val, bool valid, int len, int prec);
static void printInt(unsigned long val, bool valid, int len);
static void printDateTime(TinyGPSDate &d, TinyGPSTime &t);
static void printStr(const char *str, int len);


void setup() {
  Wire.begin(21,22);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  SerialGPS.begin(9600, SERIAL_8N1, RXD2, TXD2);

  // Struct für Zeitfunktionen
  TinyGPSTime t;
  //Struct für Datumsfunktionen
  TinyGPSDate d;

  String str ="";
  str += t.value();

  // OLED Configuration
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void loop() {
  display.clearDisplay();

  printDateTime(gps.date, gps.time);

  while (SerialGPS.available() > 0) {
      gps.encode(SerialGPS.read());

      // Struct für Zeitfunktionen
      TinyGPSTime t;
      //Struct für Datumsfunktionen
      TinyGPSDate d;

      String str ="";
      str += t.value();

      if(!t.isValid()){
        display.setCursor(15,17);
        display.println("no time received");
        display.setCursor(0,0);
        display.println("00000000000000000");
      }
      else{
        display.setCursor(15,15);
        display.println(str);
      }

  }
  /*// Struct für Zeitfunktionen
  TinyGPSTime t;
  //Struct für Datumsfunktionen
  TinyGPSDate d;

  String str ="";
  str += t.value();

  if(!t.isValid()){
    display.setCursor(15,17);
    display.println("no time received");
    display.setCursor(0,0);
    display.println("000000000");
  }
  else{
    display.setCursor(15,15);
    display.println(str);
  }*/
  display.display();
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (SerialGPS.available())
      gps.encode(SerialGPS.read());
  } while (millis() - start < ms);
}

static void printFloat(float val, bool valid, int len, int prec)
{
  if (!valid)
  {
    while (len-- > 1)
      Serial.print('*');
    Serial.print(' ');
  }
  else
  {
    Serial.print(val, prec);
    int vi = abs((int)val);
    int flen = prec + (val < 0.0 ? 2 : 1); // . and -
    flen += vi >= 1000 ? 4 : vi >= 100 ? 3 : vi >= 10 ? 2 : 1;
    for (int i=flen; i<len; ++i)
      Serial.print(' ');
  }
  smartDelay(0);
}

static void printInt(unsigned long val, bool valid, int len)
{
  char sz[32] = "*****************";
  if (valid)
    sprintf(sz, "%ld", val);
  sz[len] = 0;
  for (int i=strlen(sz); i<len; ++i)
    sz[i] = ' ';
  if (len > 0)
    sz[len-1] = ' ';
  Serial.print(sz);
  smartDelay(0);
}

static void printDateTime(TinyGPSDate &d, TinyGPSTime &t)
{
  if (!d.isValid())
  {
    Serial.print(F("********** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d/%02d/%02d ", d.month(), d.day(), d.year());
    Serial.print(sz);
  }

  if (!t.isValid())
  {
    Serial.print(F("******** "));
  }
  else
  {
    char sz[32];
    sprintf(sz, "%02d:%02d:%02d ", t.hour(), t.minute(), t.second());
    Serial.print(sz);
  }

  printInt(d.age(), d.isValid(), 5);
  smartDelay(0);
}

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}

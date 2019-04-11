#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "LM75A.h"
#include <WiFi.h>
#include "PubSubClient.h"
#include "TinyGPS++.h"

////                OLED Display                    ////
Adafruit_SSD1306 display(0);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

////                GPS-Reseiver                    ////
HardwareSerial Serial_2(2); // (2) weil UART2
//  Rx and Dx PIN-SET
#define RXD2 16
#define TXD2 17
// The TinyGPS++ object
TinyGPSPlus gps;
// GPS Functions
static void smartDelay(unsigned long ms);
static void printFloat(float val, bool valid, int len, int prec);
static void printInt(unsigned long val, bool valid, int len);
static void printDateTime(TinyGPSDate &d, TinyGPSTime &t);
//static void printStr(const char *str, int len);
//  Publishing Strings
char str_long[20];
char str_lat[20];



////               Temperature Sensor             ////
// Create I2C LM75A instance
LM75A lm75a_sensor(false,  //A0 LM75A pin state
                   false,  //A1 LM75A pin state
                   false); //A2 LM75A pin state

///   WiFi Constants
#define WIFISSID "HerzWiFi" // Put your WifiSSID here
#define PASSWORD "vohdbmrfsl9p" // Put your wifi password here
#define TOKEN "A1E-XiyvObDGuNpIeTXMjqlEiCFhpm6Qxi" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "MQTT_Client" // MQTT client Name, it should be a random and unique ascii string and different from all other devices

///   Labels for MQTT
#define VARIABLE_LABEL_TEMP "temperature" // Assing the variable label
#define VARIABLE_LABEL_LONG "long" // Assing the variable label
#define VARIABLE_LABEL_LAT "lat" // Assing the variable label
#define VARIABLE_LABEL_GPS "gps" // Assing the variable label
#define DEVICE_LABEL "esp32" // Assig the device label

////                WiFi/MQTT                   ////
///   WiFi
char mqttBroker[]  = "http://things.ubidots.com";
char payload[100];
char topic[150];
char topicSubscribe[100];
// Space to store values to send
char str_temperature[10];
// Client
WiFiClient ubidots;
PubSubClient client(ubidots);
//  Wifi/MQTT Functions
void reconnect();
void callback(char* topic, byte* payload, unsigned int length);


void testscrolltext();


void setup()   {
  Serial.begin(9600);
  Serial_2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  Wire.begin(19,23);
  WiFi.begin(WIFISSID, PASSWORD);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64, grounded)
  // init done

  Serial.println("\n\n\n**********  Welcome to sunnyHOME  ************");
  Serial.println("****** you`re personal Weather Station  ******");
  // draw scrolling text
  testscrolltext();
  display.clearDisplay();
  display.display();

  //  Connecting to WiFi
  Serial.println();
  Serial.print("Wait for WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);

  client.subscribe(topicSubscribe);

  if (!client.connected()) {
    client.subscribe(topicSubscribe);
    reconnect();
  }




}


void loop() {
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("Temperature = ");
  display.println(lm75a_sensor.getTemperatureInDegrees());
  display.println(" C");
  display.setTextSize(2);
  delay(1000);
  display.display();
  display.clearDisplay();

  ///   GPS GPS_DATA
  Serial.print("\nLat: ");
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  dtostrf(gps.location.lat(), /*11*/9, 6, str_lat);  //  for MQTT
  Serial.print("\nLong: ");
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  dtostrf(gps.location.lng(), /*12*/9, 6, str_long); //  for MQTT
  //printInt(gps.location.age(), gps.location.isValid(), 5);
  printDateTime(gps.date, gps.time);
  /*printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);*/
  Serial.println();

  Serial.println("\n\n");

  float temperature_in_degrees = lm75a_sensor.getTemperatureInDegrees();
  if (temperature_in_degrees == INVALID_LM75A_TEMPERATURE) {
      Serial.println("Error while getting temperature");
  } else {
      Serial.print("Temperature: ");
      Serial.print(temperature_in_degrees);
      Serial.print(" degrees (");
      Serial.print(LM75A::degreesToFahrenheit(temperature_in_degrees));
      Serial.println(" Fahrenheit)");
      dtostrf(temperature_in_degrees, 4, 2, str_temperature);
  }

  ///   DATA PUBLISHING

  ///   Temperature
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_TEMP); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}}", payload, str_temperature); // Adds the value
  Serial.println("\nPublishing Temperature to Ubidots Cloud");
  Serial.println(payload);
  client.publish(topic, payload);
  client.loop();
  ///   Longitude
  /*sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_LONG); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}}", payload, str_long); // Adds the value
  Serial.println("\nPublishing Temperature to Ubidots Cloud");
  client.publish(topic, payload);
  client.loop();
  ///   Latitude
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_LAT); // Adds the variable label
  sprintf(payload, "%s {\"value\": %s}}", payload, str_lat); // Adds the value
  Serial.println("\nPublishing Temperature to Ubidots Cloud");
  client.publish(topic, payload);
  client.loop();*/
  ///   GPS
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_GPS); // Adds the variable label
  sprintf(payload, "%s {\"value\":10,\"context\":{\"lat\":%s,\"lng\":%s}}}", payload, str_lat, str_long); // Adds the value
  Serial.println("\nPublishing GPS to Ubidots Cloud:");
  Serial.println(payload);
  client.publish(topic, payload);
  client.loop();

  delay(5000);


  Serial_2.flush();
  Serial.flush();
}










void testscrolltext(void) {
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(10,0);
  display.clearDisplay();
  display.println("sunnyHOME");
  display.display();
  delay(1);

  display.startscrollright(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrollleft(0x00, 0x0F);
  delay(2000);
  display.stopscroll();
  delay(1000);
  display.startscrolldiagright(0x00, 0x07);
  delay(2000);
  display.startscrolldiagleft(0x00, 0x07);
  delay(2000);
  display.stopscroll();
}



void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");

    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
      client.subscribe(topicSubscribe);
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}
void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

// This custom version of delay() ensures that the gps object
// is being "fed".
static void smartDelay(unsigned long ms)
{
  unsigned long start = millis();
  do
  {
    while (Serial_2.available())
      gps.encode(Serial_2.read());
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

/*static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}*/

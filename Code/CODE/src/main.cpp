//Standart
#include <Arduino.h>
#include <HardwareSerial.h>
#include "Wire.h"
//WiFi
#include <WiFi.h>
#include "libraries/PubSubClient.h"
//GPS
#include "libraries/TinyGPS++.h"
//Sensors
#include "libraries/LM75A.h"
#include "libraries/DS1307.h"

///   WiFi Constants
#define WIFISSID "OnePlus5" // Put your WifiSSID here
#define PASSWORD "inda.net" // Put your wifi password here
#define TOKEN "A1E-XiyvObDGuNpIeTXMjqlEiCFhpm6Qxi" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "MQTT_Client" // MQTT client Name, it should be a random and unique ascii string and different from all other devices

///   Labels
#define VARIABLE_LABEL_TEMP "temperature" // Assing the variable label
#define VARIABLE_LABEL_GPS "gps" // Assing the variable label
#define DEVICE_LABEL "esp32" // Assig the device label

///   Deep-Sleep Defines
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  15        /* Time ESP32 will go to sleep (in seconds) */

///   I2C
TwoWire i2c = TwoWire(0);

///   WiFi
char mqttBroker[]  = "things.ubidots.com";
char payload[100];
char topic[150];
char topicSubscribe[100];

// Space to store values to send
char str_temperature[10];

WiFiClient ubidots;
PubSubClient client(ubidots);

///   GPS Globals
HardwareSerial Serial_2(2);
#define RXD2 17
#define TXD2 16
// The TinyGPS++ object
TinyGPSPlus gps;
///   GPS Functions
static void smartDelay(unsigned long ms);
static void printFloat(float val, bool valid, int len, int prec);
static void printInt(unsigned long val, bool valid, int len);
static void printDateTime(TinyGPSDate &d, TinyGPSTime &t);
//static void printStr(const char *str, int len);

///   Temperature Sensor
// Create I2C LM75A instance
LM75A lm75a_sensor(false,  //A0 LM75A pin state
                   false,  //A1 LM75A pin state
                   false); //A2 LM75A pin state

///   Deep-Sleep Counter
RTC_DATA_ATTR int bootCount = 0;
// Method to print the reason by which ESP32 has been awaken from sleep
void print_wakeup_reason(){
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason)
    {
        case 1  : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
        case 2  : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
        case 3  : Serial.println("Wakeup caused by timer"); break;
        case 4  : Serial.println("Wakeup caused by touchpad"); break;
        case 5  : Serial.println("Wakeup caused by ULP program"); break;
        default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
    }
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
  if (message == "0") {
    digitalWrite(relay, LOW);
  } else {
    digitalWrite(relay, HIGH);
  }

  Serial.write(payload, length);
  Serial.println();
}


void setup(){
  Serial.begin(9600);
  Serial_2.begin(9600, SERIAL_8N1, RXD2, TXD2);
  WiFi.begin(WIFISSID, PASSWORD);
  i2c.begin(18, 19);   //SDA=18 SCL=19

  delay(1000); //Take some time to open up the Serial Monitor

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

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_TEMP); // Adds the variable label


  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("\nBoot number: " + String(bootCount));
  //Print the wakeup reason for ESP32
  print_wakeup_reason();

  //First we configure the wake up source. We set our ESP32 to wake up every 5 seconds
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) + " Seconds");


  ///   GPS GPS_DATA
  printFloat(gps.location.lat(), gps.location.isValid(), 11, 6);
  printFloat(gps.location.lng(), gps.location.isValid(), 12, 6);
  printInt(gps.location.age(), gps.location.isValid(), 5);
  printDateTime(gps.date, gps.time);
  printInt(gps.charsProcessed(), true, 6);
  printInt(gps.sentencesWithFix(), true, 10);
  printInt(gps.failedChecksum(), true, 9);
  Serial.println();

  Serial.println("\n\n");

  ///   Temperature Meausrement
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
  sprintf(payload, "%s {\"value\": %s}}", payload, str_temperature); // Adds the value
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  client.loop();
  delay(1000);

  Serial.println("\n");

  ///   Sleeping command
  Serial.println("Going to sleep now");
  Serial.println("\n");
  Serial.flush();
  esp_deep_sleep_start();
}

void loop(){
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

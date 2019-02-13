#include <Arduino.h>
#include <HardwareSerial.h>
//WiFi
#include <WiFi.h>
#include "libraries/PubSubClient.h"
//GPS
#include "libraries/TinyGPS++.h"
//Sensors
#include "libraries/LM75A.h"
#include "libraries/DS1307.h"


///   Deep-Sleep Defines
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  15        /* Time ESP32 will go to sleep (in seconds) */

///   WiFi
const char* ssid = "OnePlus 5";
const char* password = "inda.net";
const char* mqtt_server = "192.168.43.202";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
//  Functions
void setup_wifi();
void callback(char* topic, byte* message, unsigned int length);
void reconnect();


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
static void printStr(const char *str, int len);


///   Temperature Sensor
// Create I2C LM75A instance
LM75A lm75a_sensor(false,  //A0 LM75A pin state
                   false,  //A1 LM75A pin state
                   false); //A2 LM75A pin state
// Equivalent to "LM75A lm75a_sensor;"


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


void setup(){
  Serial.begin(9600);
  Serial_2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  //setup_wifi();
  //client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);

  delay(1000); //Take some time to open up the Serial Monitor

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
  }

  Serial.println("\n");

  ///   Sleeping command
  Serial.println("Going to sleep now");
  Serial.println("\n");
  Serial.flush();
  esp_deep_sleep_start();
}

void loop(){
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "esp32/output") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
      //digitalWrite(ledPin, HIGH);
    }
    else if(messageTemp == "off"){
      Serial.println("off");
      //digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
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

static void printStr(const char *str, int len)
{
  int slen = strlen(str);
  for (int i=0; i<len; ++i)
    Serial.print(i<slen ? str[i] : ' ');
  smartDelay(0);
}

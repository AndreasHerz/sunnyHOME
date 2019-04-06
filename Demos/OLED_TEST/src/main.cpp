#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include "LM75A.h"
#include <WiFi.h>
#include "PubSubClient.h"

////               OLED Display                   ////
Adafruit_SSD1306 display(0);
#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

////               Temperature Sensor             ////
// Create I2C LM75A instance
LM75A lm75a_sensor(false,  //A0 LM75A pin state
                   false,  //A1 LM75A pin state
                   false); //A2 LM75A pin state

///   WiFi Constants
#define WIFISSID "Magdis Mac" // Put your WifiSSID here
#define PASSWORD "magdiiscool" // Put your wifi password here
#define TOKEN "A1E-XiyvObDGuNpIeTXMjqlEiCFhpm6Qxi" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "MQTT_Client" // MQTT client Name, it should be a random and unique ascii string and different from all other devices

///   Labels for MQTT
#define VARIABLE_LABEL_TEMP "temperature" // Assing the variable label
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
  Wire.begin(19,23);
  WiFi.begin(WIFISSID, PASSWORD);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x64, grounded)
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);
  display.clearDisplay();

  // draw scrolling text
  testscrolltext();
  delay(2000);
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

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_TEMP); // Adds the variable label





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

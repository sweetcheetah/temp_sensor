/***************************************************
  Sensor
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

/************************* LED ***********************************/
#define RED 0
#define BLUE 2

/************************* DHT22 *********************************/

// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.
#define DHTPIN 13

//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT dht(DHTPIN, DHTTYPE);


/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "cheetah"
#define WLAN_PASS       "sweetcheetah"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "10.0.0.14"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    ""
#define AIO_KEY         ""

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe onoff = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/lights/1");

void MQTT_connect();

void setup() {
  Serial.begin(9600);
  delay(500);
  dht.begin();
  // setup LEDs
  pinMode(RED, OUTPUT);
  pinMode(BLUE, OUTPUT);
  digitalWrite(RED, HIGH);
  digitalWrite(BLUE, HIGH);
  
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  mqtt.subscribe(&onoff);
}

void loop() {
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  String message;
  
  float temp = dht.readTemperature(true);
  if (isnan(temp)) {
    Serial.println(F("Error reading temperature!"));
    message = "error reading temperature ";
  } else {
   message = "Temperature: ";
   message = message + temp;
   message = message + "F";
  }
  
  float humidity = dht.readHumidity();
  if (isnan(humidity)) {
    Serial.println(F("Error reading humidity!"));
    message = message + "error reading humidity ";
  } else {
   message = message + ", Humidity: ";
   message = message + humidity;
  }

  if (! temp_reading.publish(message.c_str())) {
    Serial.println(F("Failed"));
    digitalWrite(RED, LOW);
    delay(500);
    digitalWrite(RED, HIGH);
  } else {
    digitalWrite(BLUE, LOW);
    delay(500);
    digitalWrite(BLUE, HIGH);
  }
  
  delay(30 * 1000);
  //ESP.deepSleep(60 * 1000);

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}

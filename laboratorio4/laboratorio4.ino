#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#define WLAN_SSID "ARTEFACTOS"
#define WLAN_PASS "ARTEFACTOS"

#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_USERNAME "ErickGomezUCA"
#define AIO_KEY "aio_rEoh41f8zaJqN3Uujz9uzePSsrBZ"

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish temp = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/prueba");

Adafruit_MQTT_Subscribe slider = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/slider", MQTT_QOS_1);
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/onoff", MQTT_QOS_1);

int led = 23;
int lm35 = 34;

void slidercallback(double x) {
  Serial.print("Hey we're in a slider callback, the slider value is: ");
  Serial.println(x);
}

void onoffcallback(char *data, uint16_t len) {
  Serial.print("Hey we're in a onoff callback, the button value is: ");
  Serial.println(data);
  String message = String(data);
  message.trim();
  if (message == "ON") {
    digitalWrite(led, HIGH);
  }
  if (message == "OFF") {
    digitalWrite(led, LOW);
  }
}

void setup() {
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  slider.setCallback(slidercallback);
  onoffbutton.setCallback(onoffcallback);

  mqtt.subscribe(&slider);
  mqtt.subscribe(&onoffbutton);
}

uint32_t x = 0;

void loop() {
  MQTT_connect();

  mqtt.processPackets(10000);

  if (!mqtt.ping()) {
    mqtt.disconnect();
  }

  int lecture = analogRead(lm35);
  float voltage = lecture * (3.3 / 4095.0);
  float temperature = voltage * 100;

  Serial.print("Temperatura: ");
  Serial.print(temperature);
  Serial.print(" °C");
  Serial.println();

  Serial.print(F("\nEnviando valor a feed prueba"));
  Serial.print("...");
  if (!temp.publish(temperature)) {
    Serial.println(F("Fallido!"));
  } else {
    Serial.println(F("Éxito!"));
  }
  x++;
}

void MQTT_connect() {
  int8_t ret;
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying MQTT connection in 10 seconds...");
    mqtt.disconnect();
    delay(10000);
    retries--;
    if (retries == 0) {
      while (1);
    }
  }
  Serial.println("MQTT Connected!");
}

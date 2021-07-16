/* Libary */
#include <CO2Sensor.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

/* */
CO2Sensor co2Sensor(A0, 0.99, 100);
LiquidCrystal_I2C  lcd(0x27, 16, 2);
WiFiClient client;
PubSubClient mqtt(client);

/* Define */

/* Wifi Connect Password User */
char* ssid = "RSRC2"; //Wifi Name
char* password = "043234794"; //Wifi Password

/* MQTT Config */
const char* mqttServer = "broker.mqttdashboard.com"; //MQTT Server
const int mqttPort = 1883; //MQTT Port
const char* mqttUser = ""; //MQTT User
const char* mqttPassword = ""; //MQTT Password
char mqtt_name_id[] = ""; //ชื่อไอดี (ไม่จำเป็นต้องใส่ก็ได้)
const char* mqtt_topic = "ntnode/co2/"; // MQTT topic

/* Device ID config */
String device_id = "NTnode CO2";

/*Json config */
StaticJsonDocument<256> doc;
char buffer[256];

void setupwifi() { //Fucniton Wifi setup for easy setup wifi (Update LCD)
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Wifi Status");
  lcd.setCursor(2, 1);
  lcd.print("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Connecting");
  lcd.setCursor(6, 1);
  lcd.print("Succeed!");
  delay(2000);
  lcd.clear();
}

void setuplcd() {
  lcd.begin();
  lcd.backlight();
}

void callback (char* topic, byte* payload, unsigned int length) { //Talk with MQTT server
  deserializeJson(doc, payload, length);
  size_t n = serializeJson(doc, buffer);
  ///////////////////////////////////////////////////////////////////////////////////////////////
  if (mqtt.connect(mqtt_topic, mqttUser, mqttPassword)) {
    if (mqtt.publish(mqtt_topic, buffer, n) == true) {
      Serial.println("publish Valve status success");
    } else {
      Serial.println("publish Fail");
    }
  } else {
    Serial.println("Connect Fail MQTT");
  }
}

void reconnect_mqtt() { //Reconnect for MQTT or Wifi in ESP Down it is reconnect now
  while (!mqtt.connected()) {
    unsigned long currentMillis = millis();
    unsigned long previousMillis = 0;
    unsigned long interval = 30000;
    // if WiFi is down, try reconnecting every CHECK_WIFI_TIME seconds
    if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= interval)) {
      Serial.print(millis());
      Serial.println("Reconnecting to WiFi...");
      WiFi.disconnect();
      WiFi.reconnect();
      previousMillis = currentMillis;
    }
    Serial.print("Attempting MQTT connection...");
    reconnect_lcd_text_1();
    //WiFi.reconnect();
    if (mqtt.connect(mqtt_name_id, mqttUser, mqttPassword)) {
      Serial.println("connected");
      reconnect_lcd_text_2();
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setupnt() {
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("NT");
  lcd.setCursor(2, 1);
  lcd.print("Node CO2");
  delay(3000);
}

void reconnect_lcd_text_1() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connecting lost");
  lcd.setCursor(1, 1);
  lcd.print("Reconnect...");
}

void reconnect_lcd_text_2() {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print("Reconnect...");
  lcd.setCursor(6, 1);
  lcd.print("Succeed");
  delay(500);
  lcd.clear();
}

void calibrate_co2_lcd_cali() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrate CO2");
  lcd.setCursor(1, 1);
  lcd.print("Calibrate...");
}

void calibrate_co2_lcd_suc() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrate CO2");
  lcd.setCursor(6, 1);
  lcd.print("Succeed");
  delay(500);
}

void setup() {
  Serial.begin(9600);
  Serial.println("=== Initialized ===");
  setuplcd();
  setupnt();
  setupwifi();
  mqtt.setServer(mqttServer, mqttPort); //Login MQTT
  mqtt.setCallback(callback);
  calibrate_co2_lcd_cali();
  co2Sensor.calibrate();
  calibrate_co2_lcd_suc();
  lcd.clear();
}

void loop() {
  if (!mqtt.connected()) {
    Serial.println("---Reconnect MQTT ---");
    reconnect_mqtt();
  }
  mqtt.loop();
  ///////////////////////////////////////////////
  int val = co2Sensor.read();
  Serial.print("CO2 value: ");
  Serial.println(val);
  //////////////////////////////////////////////
  lcd.setCursor(0, 0);
  lcd.print("Carbon dioxide");
  lcd.setCursor(0, 1);
  lcd.print("Val:");
  lcd.setCursor(4, 1);
  lcd.print(val);
  lcd.setCursor(12, 1);
  lcd.print("PPM");
  //////////////////////////////////////////////
  doc["deviceid"] = device_id;
  doc["co2_value"] = val;
  //////////////////////////////////////////////
  size_t n = serializeJson(doc, buffer);

  if (mqtt.connect(mqtt_name_id, mqttUser, mqttPassword)) {
    Serial.println("\nConnected MQTT: ");
    if (mqtt.publish(mqtt_topic, buffer , n) == true) {
      Serial.println("publish success");
    } else {
      Serial.println("publish Fail");
    }
  } else {
    Serial.println("Connect Fail MQTT");
  }
  serializeJsonPretty(doc, Serial);
  //////////////////////////////////////////////
  Serial.println();
  Serial.println("-------------------------");
  delay(5000);
  lcd.clear();
}

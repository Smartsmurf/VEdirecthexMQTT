//
// This is part of the VEdirecthexMQTT project
//
// https://github.com/Smartsmurf/VEdirecthexMQTT
// 
// VE.Direct <-> MQTT bridge
// - Reads VE.Direct (UART) in text mode
// - Publishes keys (+ JSON snapshots) to MQTT
// - Subscribes to control topics and writes RAM data (SoC, Temp, Current, Voltage) to VE.Direct using Hex frames
// - Uses FreeRTOS tasks, a serial mutex, and an MQTT publish queue
  
// 
#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>
#include <WebServer.h>
// #include <ESPAsyncWebServer.h>
// #include <WebSerial.h>

//#include "felicity.h"
#include "html.h"
#include "main.h"
#include "mqtt.h"
#include "vedirect.h"
#include "preferences.h"

unsigned long lastWifiCheck = 0;

void WiFiStationDisconnected(WiFiEvent_t event, WiFiEventInfo_t info){
    lastWifiCheck = millis();
    WiFi.begin(ssid.c_str(), password.c_str());
}

void setup() {

  Serial.begin(9600);
  Serial.println("Starting up...");

  loadSettings();

  WiFi.onEvent(WiFiStationDisconnected, WiFiEvent_t::ARDUINO_EVENT_WIFI_STA_DISCONNECTED);

  if (ssid != "") {
    WiFi.begin(ssid.c_str(), password.c_str());
    WiFi.setAutoReconnect(true);
    Serial.print("Trying to connect to SSID: ");
    Serial.println(ssid);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\n✅ WIFI connected.");
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      startWebServer();

//      xTaskCreatePinnedToCore(FelicityBMS::bmsTaskWrapper, "BMS", 4096, bms, 1, NULL, 1);
//      xTaskCreatePinnedToCore(mqtt_task, "MQTT", 4096, NULL, 1, NULL, 1);

    Serial.begin(115200); delay(100); Serial.println("VE.Direct <-> MQTT bridge starting...");
    Serial2.begin(VE_BAUD,SERIAL_8N1,VE_RX_PIN,VE_TX_PIN);
    mqttPublishQueue=xQueueCreate(PUB_QUEUE_LEN,sizeof(MqttMessage));
    vedirectWriteQueue=xQueueCreate(8,256);
    serialMutex=xSemaphoreCreateMutex();


    xTaskCreatePinnedToCore(vedirectTask,"vedirectTask",4096,NULL,2,NULL,1);
    xTaskCreatePinnedToCore(mqttTask,"mqttTask",8192,NULL,1,NULL,0);

      Serial.println("System started.");

      analogWrite(LED_PIN, 255);

      return;
    } else {
      Serial.println("\n⚠️ WIFI connection failed.");
    }
  }

  // no WIFI - run config AP
  startConfigPortal();

}

void loop() {
  
  // check WIFI state
  unsigned long now = millis();
  if (now - lastWifiCheck >= 10000) {
    lastWifiCheck = now;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WIFI disconnected! Trying reconnect...");

      WiFi.disconnect();  // sanity
      WiFi.begin(ssid.c_str(), password.c_str());
    }
  }

  server.handleClient();

  if (!mqtt.connected()) reconnectMQTT();
  // mqtt.loop();
  // readVEHexPacket();

}

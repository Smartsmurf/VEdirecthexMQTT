#include <Arduino.h>
#include <PubSubClient.h>


void reconnectMQTT() {
  while (!mqtt.connected()) {
    Serial.print("[MQTT] Verbindung...");
    if (mqtt.connect("ESP32-VEHex")) {
      Serial.println("erfolgreich");
      mqtt.subscribe(topic_sub);
    } else {
      Serial.print("Fehler, rc=");
      Serial.print(mqtt.state());
      delay(3000);
    }
  }
}
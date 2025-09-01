//
// This is part of the VEdirecthexMQTT project
//
// https://github.com/Smartsmurf/VEdirecthexMQTT
// 
#include "preferences.h"

Preferences prefs;

String ssid, password, mqttServer, mqttUser, mqttPass, mqttTopic, mqttDevicename;
int rxPin, txPin, mqttPort;

// String subscrTopicSoc, subscrTopicTemp, subscrTopicCurrent, subscrTopicVoltage;
// String subscrTopic[MAX_SUBSCRIPTION_TOPICS];

void loadSettings() {
  prefs.begin("config", true);  // read-only
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("pass", "");
  mqttServer = prefs.getString("mqtt", "");
  mqttPort = prefs.getInt("mqtt_port", 1883);
  mqttTopic = prefs.getString("mqtt_topic", "vedirect");
  mqttUser = prefs.getString("mqtt_user", "");
  mqttPass = prefs.getString("mqtt_pass", "");
  mqttDevicename = prefs.getString("mqtt_devicename", "ve2mqtt");
  rxPin = prefs.getInt("rx_pin", 16);
  txPin = prefs.getInt("tx_pin", 17);
  for( int i = 0; i < MAX_SUBSCRIPTION_TOPICS; i++ ){
    VEregisters[i].mqtt_subscription = prefs.getString((String("mqtt_subscription_") + String(i)).c_str(), "");
  }
//  rtsPin = prefs.getInt("rts_pin", 4);
//  batteryCount = prefs.getInt("batt_count", 1);
  prefs.end();
}

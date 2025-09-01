//
// This is part of the VEdirecthexMQTT project
//
// https://github.com/Smartsmurf/VEdirecthexMQTT
// 
#ifndef VDHXMQTT_PREFS_H
#define VDHXMQTT_PREFS_H

#include <Arduino.h>
#include <Preferences.h>

#include "vedirect.h"

#define MAX_SUBSCRIPTION_TOPICS NUM_VE_HEX_REGISTERS

extern String ssid, password, mqttServer, mqttUser, mqttPass, mqttTopic, mqttDevicename;
// extern String subscrTopicSoc, subscrTopicTemp, subscrTopicCurrent, subscrTopicVoltage;
// extern String subscrTopic[MAX_SUBSCRIPTION_TOPICS];
extern int rxPin, txPin, mqttPort;
extern Preferences prefs;

void loadSettings();

#endif

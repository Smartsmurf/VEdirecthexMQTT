//
// This is part of the VEdirecthexMQTT project
//
// https://github.com/Smartsmurf/VEdirecthexMQTT
// 
#ifndef VDHXMQTT_MQTT_H
#define VDHXMQTT_MQTT_H

#include <Arduino.h>
#include <PubSubClient.h>

#define PUB_QUEUE_LEN 16
#define PUB_TOPIC_MAX 128
#define PUB_PAYLOAD_MAX 512

typedef struct MqttMessage {
  char topic[PUB_TOPIC_MAX];
  char payload[PUB_PAYLOAD_MAX];
  uint8_t qos;
  bool retain;
} MqttMessage_t; 

extern QueueHandle_t mqttPublishQueue;
extern PubSubClient mqtt;


void mqtt_publishkey(const char* key, const char* value);
void mqttCallback(char* topic, byte* payload, unsigned int length);
void mqttTask(void *pvParameters);
void reconnectMQTT(void);

#endif // VDHXMQTT_MQTT_H
//
// This is part of the VEdirecthexMQTT project
//
// https://github.com/Smartsmurf/VEdirecthexMQTT
// 
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "main.h"
#include "mqtt.h"
#include "preferences.h"
#include "vedirect.h"

// MQTT
WiFiClient espClient;
PubSubClient mqtt(espClient);

QueueHandle_t mqttPublishQueue;

static void mqtt_publish(const char* topic, const char* payload, uint8_t qos=0, bool retain=false) {
  MqttMessage_t msg;
  strncpy(msg.topic, topic, PUB_TOPIC_MAX-1); msg.topic[PUB_TOPIC_MAX-1]='\0';
  strncpy(msg.payload, payload, PUB_PAYLOAD_MAX-1); msg.payload[PUB_PAYLOAD_MAX-1]='\0';
  msg.qos=qos; msg.retain=retain;
  if(xQueueSend(mqttPublishQueue,&msg,0)!=pdTRUE){
    MqttMessage_t tmp;
    xQueueReceive(mqttPublishQueue,&tmp,0); 
    xQueueSend(mqttPublishQueue,&msg,0);
  }
}   

void mqtt_publishkey(const char* key, const char* value){
  char topic[PUB_TOPIC_MAX];
  snprintf(topic,sizeof(topic),"%s/%s/text/%s",mqttTopic.c_str(),mqttDevicename.c_str(),key);
  mqtt_publish(topic,value,0,false);
}

void reconnectMQTT(void) {
  while (!mqtt.connected()) {
    Serial.print("[MQTT] Verbindung...");
    if (mqtt.connect("ESP32-VEHex")) {
      Serial.println("erfolgreich");
      // mqtt.subscribe(topic_sub);
    } else {
      Serial.print("Fehler, rc=");
      Serial.print(mqtt.state());
      delay(3000);
    }
  }
}

bool mqttConnect(void)
{
  char mqttClientId[80];
  if(WiFi.status()!=WL_CONNECTED)
    return false;
  if(mqtt.connected())
    return true;
//  String clientId=String("vedirect-")+DEVICE_ID+String("-")+String((uint32_t)esp_random());
  uint32_t chipId = (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF);
  sprintf(mqttClientId, "%s-%06X", mqttDevicename.c_str(), chipId);
  
  mqtt.setServer(mqttServer.c_str(),mqttPort);
  mqtt.setCallback(mqttCallback);
  if(mqttServer.length()>0){
    if(mqtt.connect(mqttClientId,mqttUser.c_str(),mqttPass.c_str())){
      char sub[128];
      snprintf(sub,sizeof(sub),"%s/%s/cmd/#",mqttTopic.c_str(),mqttDevicename.c_str());
      mqtt.subscribe(sub);
      char st[128]; snprintf(st,sizeof(st),"%s/%s/status",mqttTopic.c_str(),mqttDevicename.c_str());
      mqtt.publish(st,"online",true);
      return true;
    }
  } else {
    if(mqtt.connect(mqttClientId)){
      char sub[128]; snprintf(sub,sizeof(sub),"%s/%s/cmd/#",mqttTopic.c_str(),mqttDevicename.c_str()); mqtt.subscribe(sub);
      char st[128]; snprintf(st,sizeof(st),"%s/%s/status",mqttTopic.c_str(),mqttDevicename.c_str()); mqtt.publish(st,"online",true);
      return true;
    }
  }
  return false;
}   

void handleMqttMessage(char* topic, byte* payload, unsigned int){
  char prefix[128];
  
  snprintf(prefix,sizeof(prefix),"%s/%s/cmd/",mqttTopic.c_str(),mqttDevicename.c_str());
  if(strncmp(topic,prefix,strlen(prefix))!=0) return;

  // Payload format: KEY:VALUE e.g., SOC:80
  char* sep = strchr((char*)payload,':');
  if(!sep) return;
  *sep='\0'; char* key=(char*)payload; char* valStr=sep+1; int val=atoi(valStr);

  // Only allow known keys
  if(strcmp(key,"SOC")!=0 && strcmp(key,"TEMP")!=0 && strcmp(key,"CURRENT")!=0 && strcmp(key,"VOLTAGE")!=0) return;

  // Send as hex command
  sendHexCommand(key,val);
}
   

void mqttCallback(char* topic, byte* payload, unsigned int length){
  char buf[512]; if(length>=sizeof(buf)) length=sizeof(buf)-1;
  memcpy(buf,payload,length); buf[length]='\0';
  handleMqttMessage(topic,(byte*)buf,length);
}

void mqttTask(void *pvParameters){
  (void)pvParameters; MqttMessage msg; unsigned long lastReconnect=0;
  while(true){
    if(!mqtt.connected()){unsigned long now=millis(); if(now-lastReconnect>5000){Serial.println("Attempting MQTT connect..."); if(mqttConnect()) Serial.println("MQTT connected"); else Serial.println("MQTT connect failed"); lastReconnect=now;}}
    else{
      if(xQueueReceive(mqttPublishQueue,&msg,pdMS_TO_TICKS(100))==pdTRUE){
        bool ok=mqtt.publish(msg.topic,msg.payload,msg.retain);
        if(!ok){Serial.printf("MQTT publish failed: %s\n",msg.topic); xQueueSend(mqttPublishQueue,&msg,0);}}
      mqtt.loop();
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}  

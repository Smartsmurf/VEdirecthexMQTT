//
// This is part of the VEdirecthexMQTT project
//
// https://github.com/Smartsmurf/VEdirecthexMQTT
// 
#include <Arduino.h>
#include <WiFi.h>

#include "main.h"
#include "vedirect.h"
#include "mqtt.h"

QueueHandle_t vedirectWriteQueue;
SemaphoreHandle_t serialMutex;

VEdirectBridge_t VEregisters[NUM_VE_HEX_REGISTERS] =
{
  {VE_HEX_CHARGE_VOLTAGE_SETPOINT,    0x2001, VE_TYPE_UINT16, "Charge Voltage Setpoint", ""},
  {VE_HEX_BATTERY_VOLTAGE_SENSE,      0x2002, VE_TYPE_UINT16, "Battery Voltage Sense", ""},
  {VE_HEX_BATTERY_TEMPERATURE_SENSE,  0x2003, VE_TYPE_INT16,  "Battery Temperature Sense", ""},
  {VE_HEX_TOTAL_CHARGE_CURRENT,       0x2013, VE_TYPE_INT16,  "Total Charge Current", ""},
  {VE_HEX_CHARGE_CURRENT_LIMIT,       0x2015, VE_TYPE_UINT16, "Charge Current Limit", ""},
};


void sendHexCommand(const char* key,int value){
  uint8_t frame[8]; // Beispiel-Frame: [Header][Key][Value][Checksum]
  frame[0] = 0xA5; // Beispiel-Header

  // Bestimmen des Key-Codes basierend auf dem Schlüssel
  if (strcmp(key, "SOC") == 0) frame[1] = 0x01;
  else if (strcmp(key, "TEMP") == 0) frame[1] = 0x02;
  else if (strcmp(key, "CURRENT") == 0) frame[1] = 0x03;
  else if (strcmp(key, "VOLTAGE") == 0) frame[1] = 0x04;

  // Wert im Little Endian-Format (2 Bytes)
  frame[2] = value & 0xFF;        // Niedriges Byte
  frame[3] = (value >> 8) & 0xFF; // Hohes Byte

  // Reservierte Bytes
  frame[4] = 0;
  frame[5] = 0;
  frame[6] = 0;

  // Berechnung der Checksumme: Summe aller Bytes muss 0x55 ergeben
  uint8_t checksum = 0x55;
  for (int i = 0; i < 7; i++) {
    checksum -= frame[i];
  }
  frame[7] = checksum;

  // Senden des Frames über Serial2
  if (xSemaphoreTake(serialMutex, pdMS_TO_TICKS(200))) {
    Serial2.write(frame, sizeof(frame));
    xSemaphoreGive(serialMutex);
    Serial.printf("Sent hex frame to VE.Direct: key=%s value=%d\n",key,value);
} else Serial.println("Could not acquire serial mutex to write hex frame");
}

void vedirectTask( void *pvParameters )
{

  String line; 
//  StaticJsonDocument<1024> doc;

  while(true){
    char writeBuf[256];
    if(xQueueReceive(vedirectWriteQueue,writeBuf,0)==pdTRUE){
      if(xSemaphoreTake(serialMutex,pdMS_TO_TICKS(200))){
        Serial2.print(writeBuf); Serial2.print("\r\n");
        xSemaphoreGive(serialMutex);
        Serial.printf("WROTE to VE.Direct: %s\n",writeBuf);
      } else {
        Serial.println("Could not acquire serial mutex to write");
      }
    }
    while(Serial2.available()){
      int c=Serial2.read(); 
      if(c<0) break; 
      char ch=(char)c; 
      if(ch=='\r') 
        continue; 
      if(ch=='\n'){
        if(line.length()==0){

        }else{
          int tab=line.indexOf('\t'); 
          String k,v; 
          if(tab>=0){
            k=line.substring(0,tab);
            v=line.substring(tab+1);
          } else {
            int sp=line.indexOf(' '); 
            if(sp>=0){
              k=line.substring(0,sp);
              v=line.substring(sp+1);
            } else{
              k="_raw";v=line;
            }
          } 
//          doc[k.c_str()]=v.c_str(); 
          mqtt_publishkey(k.c_str(),v.c_str()); 
//          if(k.equalsIgnoreCase("Checksum")){
//            publishSnapshot(doc.as<JsonObject>()); 
//            doc.clear();
//          } 
          line="";
        }
      } else{
          line+=ch;
          if(line.length()>512) 
          line=line.substring(line.length()-512);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

// todo:

void writeToRegister(int index, int raw_value) {
    switch (VEregisters[index].data_type) {
        case VE_TYPE_UINT16: {
            uint16_t val = static_cast<uint16_t>(raw_value);
            // schreibe val an VEregisters[index].register_id
            break;
        }
        case VE_TYPE_INT16: {
            int16_t val = static_cast<int16_t>(raw_value);
            // schreibe val an VEregisters[index].register_id
            break;
        }
        default:
            // Fehlerbehandlung
            break;
    }
}

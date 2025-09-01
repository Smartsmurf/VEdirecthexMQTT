//
// This is part of the VEdirecthexMQTT project
//
// https://github.com/Smartsmurf/VEdirecthexMQTT
// 
#ifndef VDHXMQTT_VEDIRECT_H
#define VDHXMQTT_VEDIRECT_H

#include <Arduino.h>

#define VE_RX_PIN 16
#define VE_TX_PIN 17
#define VE_BAUD 19200        

#define VE_SERIAL Serial2

extern QueueHandle_t vedirectWriteQueue;
extern SemaphoreHandle_t serialMutex;

void sendHexCommand(const char* key,int value);
void vedirectTask( void *pvParameters );

#define NUM_VE_HEX_REGISTERS 5

enum VEdirectDataType {
    VE_TYPE_UINT8,
    VE_TYPE_INT8,
    VE_TYPE_UINT16,
    VE_TYPE_INT16
};

enum VEdirectRegisterType {
    VE_HEX_CHARGE_VOLTAGE_SETPOINT,
    VE_HEX_BATTERY_VOLTAGE_SENSE,
    VE_HEX_BATTERY_TEMPERATURE_SENSE,
    VE_HEX_TOTAL_CHARGE_CURRENT,
    VE_HEX_CHARGE_CURRENT_LIMIT    
};
    
typedef struct VEdirectBridge {
    VEdirectRegisterType register_type;
    uint16_t register_id;
    VEdirectDataType data_type;
    const char * register_name;
    String mqtt_subscription;
} VEdirectBridge_t;

extern VEdirectBridge_t VEregisters[NUM_VE_HEX_REGISTERS];

#endif // VDHXMQTT_VEDIRECT_H

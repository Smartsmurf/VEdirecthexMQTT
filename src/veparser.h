#ifndef VEPARSER_H
#define VEPARSER_H

#include <Arduino.h>

// 2-Byte Big-Endian signed int
inline int16_t be16s(const uint8_t* data) {
  return (int16_t)((data[0] << 8) | data[1]);
}

// 2-Byte Big-Endian unsigned int
inline uint16_t be16u(const uint8_t* data) {
  return ((data[0] << 8) | data[1]);
}

// 4-Byte Big-Endian signed int
inline int32_t be32s(const uint8_t* data) {
  return (int32_t)(
      ((uint32_t)data[0] << 24) |
      ((uint32_t)data[1] << 16) |
      ((uint32_t)data[2] << 8) |
      data[3]);
}

// 4-Byte Big-Endian unsigned int
inline uint32_t be32u(const uint8_t* data) {
  return ((uint32_t)data[0] << 24) |
         ((uint32_t)data[1] << 16) |
         ((uint32_t)data[2] << 8) |
         data[3];
}

// Helper: mV → float V
inline float mV_to_V(int16_t mv) {
  return mv / 1000.0f;
}

// Helper: mA → float A
inline float mA_to_A(int16_t ma) {
  return ma / 1000.0f;
}

// Example: Parse battery voltage (2 bytes, mV → float V)
inline float parseBatteryVoltage(const uint8_t* data) {
  return mV_to_V(be16s(data));
}

// Example: Parse current (2 bytes, mA → float A)
inline float parseCurrent(const uint8_t* data) {
  return mA_to_A(be16s(data));
}

// Example: Parse energy (Wh)
inline float parseEnergy(const uint8_t* data) {
  return be32s(data) * 0.01f;
}

#endif // VEPARSER_H


#pragma once
#include <Arduino.h>
#include "esphome/core/component.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/core/hal.h"

// Note: Using legacy ADC driver to maintain compatibility with epdiy library
// which also uses the legacy driver. Mixing new and legacy drivers causes conflicts.
#include <driver/adc.h>
#include "esp_adc_cal.h"

#include "epdiy.h"

namespace esphome {
namespace lilygo_t5_47_battery {

class Lilygot547Battery : public PollingComponent {
 public:
  sensor::Sensor *voltage{nullptr};

  void setup() override;
  void update() override;

  void set_voltage_sensor(sensor::Sensor *voltage_sensor) { voltage = voltage_sensor; }

 protected:
  int vref_{1100};
  void update_battery_voltage_();
  void calibrate_adc_();
};
}  // namespace lilygo_t5_47_battery
}  // namespace esphome

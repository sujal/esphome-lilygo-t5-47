#include "Lilygot547Battery.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lilygo_t5_47_battery {

static const char *const TAG = "lilygo_t5_47_battery";

void Lilygot547Battery::setup() {
  // Configure ADC1 using legacy API (compatible with epdiy)
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);

  // Calibrate ADC using eFuse vref if available
  this->calibrate_adc_();
}

void Lilygot547Battery::update() {
  epd_poweron();
  // Wait for voltage to stabilize
  delay(100);
  this->update_battery_voltage_();
  epd_poweroff();
}

void Lilygot547Battery::update_battery_voltage_() {
  // GPIO36 is ADC1_CHANNEL_0 - use legacy adc1_get_raw() to avoid conflict
  // with epdiy's use of legacy ADC driver (Arduino's analogRead uses new driver)
  int raw = adc1_get_raw(ADC1_CHANNEL_0);
  if (raw < 0) {
    ESP_LOGW(TAG, "Failed to read ADC");
    return;
  }
  // Convert to voltage: 12-bit ADC, voltage divider (x2), calibrated vref
  float battery_voltage = ((float) raw / 4095.0f) * 2.0f * 3.3f * ((float) this->vref_ / 1000.0f);

  if (this->voltage != nullptr) {
    this->voltage->publish_state(battery_voltage);
  }
}

void Lilygot547Battery::calibrate_adc_() {
  // Use eFuse calibration if available
  esp_adc_cal_characteristics_t adc_chars;
  esp_adc_cal_value_t val_type =
      esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars);
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
    this->vref_ = adc_chars.vref;
    ESP_LOGI(TAG, "Using eFuse Vref: %d mV", this->vref_);
  } else {
    ESP_LOGW(TAG, "Using default Vref: %d mV", this->vref_);
  }
}

}  // namespace lilygo_t5_47_battery
}  // namespace esphome

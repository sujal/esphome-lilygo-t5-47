#include "Lilygot547Battery.h"
#include "esphome/core/log.h"

namespace esphome {
namespace lilygo_t5_47_battery {

static const char *const TAG = "lilygo_t5_47_battery";

// GPIO36 is ADC1_CHANNEL_0 on ESP32
static const adc_channel_t BATTERY_ADC_CHANNEL = ADC1_CHANNEL_0;
static const adc_atten_t BATTERY_ADC_ATTEN = ADC_ATTEN_DB_12;

void Lilygot547Battery::setup() {
  // Initialize ADC oneshot driver
  adc_oneshot_unit_init_cfg_t init_config = {
      .unit_id = ADC_UNIT_1,
      .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  esp_err_t ret = adc_oneshot_new_unit(&init_config, &this->adc_handle_);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize ADC unit: %s", esp_err_to_name(ret));
    return;
  }

  // Configure ADC channel
  adc_oneshot_chan_cfg_t chan_config = {
      .atten = BATTERY_ADC_ATTEN,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ret = adc_oneshot_config_channel(this->adc_handle_, BATTERY_ADC_CHANNEL, &chan_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure ADC channel: %s", esp_err_to_name(ret));
    return;
  }

  // Initialize calibration (try curve fitting first, then line fitting)
#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
  adc_cali_curve_fitting_config_t cali_config = {
      .unit_id = ADC_UNIT_1,
      .chan = BATTERY_ADC_CHANNEL,
      .atten = BATTERY_ADC_ATTEN,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ret = adc_cali_create_scheme_curve_fitting(&cali_config, &this->cali_handle_);
  if (ret == ESP_OK) {
    this->calibration_enabled_ = true;
    ESP_LOGI(TAG, "ADC calibration enabled (curve fitting)");
  }
#elif ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
  adc_cali_line_fitting_config_t cali_config = {
      .unit_id = ADC_UNIT_1,
      .atten = BATTERY_ADC_ATTEN,
      .bitwidth = ADC_BITWIDTH_12,
  };
  ret = adc_cali_create_scheme_line_fitting(&cali_config, &this->cali_handle_);
  if (ret == ESP_OK) {
    this->calibration_enabled_ = true;
    ESP_LOGI(TAG, "ADC calibration enabled (line fitting)");
  }
#endif

  if (!this->calibration_enabled_) {
    ESP_LOGW(TAG, "ADC calibration not available, using raw values");
  }
}

void Lilygot547Battery::update() {
  if (this->adc_handle_ == nullptr) {
    ESP_LOGW(TAG, "ADC not initialized");
    return;
  }

  epd_poweron();
  // Wait for voltage to stabilize
  delay(100);

  int raw_value = 0;
  esp_err_t ret = adc_oneshot_read(this->adc_handle_, BATTERY_ADC_CHANNEL, &raw_value);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to read ADC: %s", esp_err_to_name(ret));
    epd_poweroff();
    return;
  }

  float battery_voltage;
  if (this->calibration_enabled_ && this->cali_handle_ != nullptr) {
    int voltage_mv = 0;
    ret = adc_cali_raw_to_voltage(this->cali_handle_, raw_value, &voltage_mv);
    if (ret == ESP_OK) {
      // Voltage divider: actual voltage = measured * 2
      battery_voltage = (voltage_mv * 2.0f) / 1000.0f;
    } else {
      // Fallback to raw calculation
      battery_voltage = ((float) raw_value / 4095.0f) * 2.0f * 3.3f;
    }
  } else {
    // Raw calculation without calibration
    battery_voltage = ((float) raw_value / 4095.0f) * 2.0f * 3.3f;
  }

  epd_poweroff();

  if (this->voltage != nullptr) {
    this->voltage->publish_state(battery_voltage);
  }
}

}  // namespace lilygo_t5_47_battery
}  // namespace esphome

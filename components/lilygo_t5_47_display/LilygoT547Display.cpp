#include "LilygoT547Display.h"
#include "esphome/core/log.h"
#include "esp_task_wdt.h"

namespace esphome {
namespace lilygo_t5_47_display {

static const char *const TAG = "lilygo_t5_47_display";

float LilygoT547Display::get_setup_priority() const { return esphome::setup_priority::LATE; }

void LilygoT547Display::set_clear_screen(bool clear) { this->clear_ = clear; }
void LilygoT547Display::set_power_off_delay_enabled(bool power_off_delay_enabled) {
  this->power_off_delay_enabled_ = power_off_delay_enabled;
}
void LilygoT547Display::set_landscape(bool landscape) { this->landscape_ = landscape; }

void LilygoT547Display::set_temperature(uint32_t temperature) { this->temperature_ = temperature; }
void LilygoT547Display::set_full_clear_on_boot(bool full_clear_on_boot) {
  this->full_clear_on_boot_ = full_clear_on_boot;
}

int LilygoT547Display::get_width_internal() { return 960; }

int LilygoT547Display::get_height_internal() { return 540; }

void LilygoT547Display::setup() {
  ESP_LOGI(TAG, "Setup: clear_=%s, landscape_=%s, temperature_=%d",
           this->clear_ ? "true" : "false",
           this->landscape_ ? "true" : "false",
           this->temperature_);

  epd_init(&epd_board_lilygo_t5_47, &ED047TC1, EPD_OPTIONS_DEFAULT);
  hl = epd_hl_init(EPD_BUILTIN_WAVEFORM);
  if (landscape_) {
    epd_set_rotation(EPD_ROT_LANDSCAPE);
  } else {
    epd_set_rotation(EPD_ROT_PORTRAIT);
  }
  fb = epd_hl_get_framebuffer(&hl);
}

void LilygoT547Display::update() {
  ESP_LOGD(TAG, "update() called: init_clear_executed_=%s, clear_=%s, full_clear_on_boot_=%s",
           this->init_clear_executed_ ? "true" : "false",
           this->clear_ ? "true" : "false",
           this->full_clear_on_boot_ ? "true" : "false");

  // Full clear on boot - uses epd_fullclear() which properly syncs framebuffers
  // This should be done BEFORE the regular clear to establish a known baseline
  if (this->full_clear_on_boot_executed_ == false && this->full_clear_on_boot_ == true) {
    ESP_LOGI(TAG, "Performing full clear on boot (syncs framebuffers with display)");
    // Disable watchdog for this task during long-running display operation
    esp_task_wdt_delete(NULL);
    epd_poweron();
    epd_fullclear(&hl, this->temperature_);
    delay(500);  // Allow display to settle
    epd_poweroff();
    esp_task_wdt_add(NULL);
    this->full_clear_on_boot_executed_ = true;
    this->just_cleared_ = true;  // Use optimized mode for next update
    ESP_LOGI(TAG, "Full clear on boot completed");
  }

  if (this->init_clear_executed_ == false && this->clear_ == true) {
    ESP_LOGI(TAG, "Triggering initial clear from update()");
    LilygoT547Display::full_clear();
    this->init_clear_executed_ = true;
  }

  this->do_update_();
  LilygoT547Display::flush_screen_changes();
}

void LilygoT547Display::full_clear() {
  ESP_LOGI(TAG, "Full hardware clear (e-paper flash)...");
  // Disable watchdog for this task during long-running display operation
  esp_task_wdt_delete(NULL);

  epd_poweron();
  epd_fullclear(&hl, this->temperature_);
  // Allow display to settle after clear
  delay(500);
  epd_poweroff();

  // Mark that we just cleared - next update can use optimized white-to-content mode
  this->just_cleared_ = true;

  // Re-enable watchdog
  esp_task_wdt_add(NULL);
  ESP_LOGI(TAG, "Full hardware clear completed");
}

void LilygoT547Display::flush_screen_changes() {
  // Disable watchdog for this task during long-running display operation
  esp_task_wdt_delete(NULL);

  epd_poweron();

  // After a full clear, use optimized white-to-content mode for better contrast
  // Otherwise use GL16 mode for non-flashing grayscale transitions (better for partial updates)
  enum EpdDrawMode mode = MODE_GL16;
  if (this->just_cleared_) {
    ESP_LOGD(TAG, "Using MODE_EPDIY_WHITE_TO_GL16 after clear for better contrast");
    mode = MODE_EPDIY_WHITE_TO_GL16;
    this->just_cleared_ = false;
  }

  err = epd_hl_update_screen(&hl, mode, this->temperature_);

  // E-paper needs time to settle after update before power off
  if (this->power_off_delay_enabled_ == true) {
    delay(500);
  }
  epd_poweroff();

  // Re-enable watchdog
  esp_task_wdt_add(NULL);
}

void LilygoT547Display::set_all_white() { epd_hl_set_all_white(&hl); }
void LilygoT547Display::poweron() { epd_poweron(); }
void LilygoT547Display::poweroff() { epd_poweroff(); }

void LilygoT547Display::on_shutdown() {
  ESP_LOGI(TAG, "Shutting down Lilygo T5-4.7 screen");
  epd_poweroff();
  epd_deinit();
}

void HOT LilygoT547Display::draw_absolute_pixel_internal(int x, int y, Color color) {
  if (color.red == 255 && color.green == 255 && color.blue == 255) {
    epd_draw_pixel(x, y, 0, fb);
  } else {
    int col = (0.2126 * color.red) + (0.7152 * color.green) + (0.0722 * color.blue);
    int cl = 255 - col;
    epd_draw_pixel(x, y, cl, fb);
  }
}

}  // namespace lilygo_t5_47_display
}  // namespace esphome

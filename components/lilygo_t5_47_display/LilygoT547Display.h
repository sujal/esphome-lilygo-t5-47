#pragma once
#include "esphome/core/component.h"

#include "esphome/components/display/display_buffer.h"
#include "esphome/components/display/display_color_utils.h"
#include "esphome/core/hal.h"

#include "epdiy.h"
#include "epd_highlevel.h"
#include "epd_board.h"

namespace esphome {
namespace lilygo_t5_47_display {

// LilyGo-EPD47
class LilygoT547Display : public display::DisplayBuffer {
 protected:
  EpdiyHighlevelState hl;
  // ambient temperature around device
  uint8_t *fb;
  enum EpdDrawError err;

 public:
  float get_setup_priority() const override;

  void set_clear_screen(bool clear);
  void set_landscape(bool landscape);
  void set_power_off_delay_enabled(bool power_off_delay_enabled);
  void set_temperature(uint32_t temperature);
  void set_full_clear_on_boot(bool full_clear_on_boot);

  int get_width_internal();

  int get_height_internal();

  void setup() override;

  void update() override;

  void full_clear();
  void flush_screen_changes();
  void set_all_white();
  void poweron();
  void poweroff();
  void on_shutdown() override;

  display::DisplayType get_display_type() override { return display::DisplayType::DISPLAY_TYPE_GRAYSCALE; }

 protected:
  void HOT draw_absolute_pixel_internal(int x, int y, Color color) override;

  bool clear_;
  bool init_clear_executed_ = false;
  bool full_clear_on_boot_;
  bool full_clear_on_boot_executed_ = false;
  uint32_t temperature_;
  bool power_off_delay_enabled_;
  bool landscape_;
};

}  // namespace lilygo_t5_47_display
}  // namespace esphome

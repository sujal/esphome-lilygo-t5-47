# ESPHome LilyGo T5 4.7" Components

[ESPHome](https://esphome.io/) external components for the LilyGo T5 4.7" e-paper display (original ESP32 version).

## Components

| Component | Description |
|-----------|-------------|
| `lilygo_t5_47_display` | E-paper display driver using the [epdiy](https://github.com/vroland/epdiy) library |
| `lilygo_t5_47_battery` | Battery voltage sensor via ADC |

## Requirements

- ESPHome 2024.1.0 or later
- LilyGo T5 4.7" (original ESP32 version with ED047TC1 display)

## Installation

Add the following to your ESPHome configuration:

```yaml
external_components:
  - source: github://sujal/esphome-lilygo-t5-47
    components: [lilygo_t5_47_display, lilygo_t5_47_battery]
```

For local development:

```yaml
external_components:
  - source:
      type: local
      path: /path/to/esphome-lilygo-t5-47/components
    components: [lilygo_t5_47_display, lilygo_t5_47_battery]
```

## Configuration

### Display Component

```yaml
display:
  - platform: lilygo_t5_47_display
    id: t5_display
    clear: true
    temperature: 25
    landscape: true
    power_off_delay_enabled: false
    lambda: |-
      it.print(100, 100, id(my_font), "Hello World!");
```

#### Configuration Variables

| Variable | Type | Default | Description |
|----------|------|---------|-------------|
| `clear` | boolean | `true` | Clear screen on first update |
| `temperature` | int | `25` | Ambient temperature (°C) for waveform timing |
| `landscape` | boolean | `true` | `true` = 960x540, `false` = 540x960 |
| `power_off_delay_enabled` | boolean | `false` | Add 700ms delay before power off |
| `lambda` | lambda | - | Drawing code |

### Battery Sensor Component

```yaml
sensor:
  - platform: lilygo_t5_47_battery
    voltage:
      name: "Battery Voltage"
```

## Complete Example

```yaml
esphome:
  name: lilygo-epaper
  platform: ESP32
  board: esp32dev

external_components:
  - source: github://sujal/esphome-lilygo-t5-47
    components: [lilygo_t5_47_display, lilygo_t5_47_battery]

wifi:
  ssid: !secret wifi_ssid
  password: !secret wifi_password

font:
  - file: "gfonts://Roboto"
    id: roboto
    size: 48

display:
  - platform: lilygo_t5_47_display
    id: t5_display
    clear: true
    temperature: 25
    landscape: true
    lambda: |-
      it.print(100, 100, id(roboto), "Hello from ESPHome!");

sensor:
  - platform: lilygo_t5_47_battery
    voltage:
      name: "Battery Voltage"
```

## Hardware

Supported hardware:
- **LilyGo T5 4.7"** (original ESP32 version)
  - MCU: ESP32 with PSRAM
  - Display: ED047TC1 (960x540, 4.7" diagonal)

This component does **not** support the newer ESP32-S3 variant.

## Credits

- Original components by [vbaksa](https://github.com/vbaksa/esphome)
- E-paper driver: [vroland/epdiy](https://github.com/vroland/epdiy)

## License

This project follows the same licensing as ESPHome:
- **Python code** (`.py` files): MIT License
- **C++ code** (`.h`, `.cpp` files): GPLv3

See [LICENSE](LICENSE) for details.

## Contributing

Contributions are welcome! Please open an issue or pull request.

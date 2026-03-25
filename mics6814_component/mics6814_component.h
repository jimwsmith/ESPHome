#pragma once

#include "esphome.h"
#include "esphome/components/sensor/sensor.h"
#include "esphome/components/i2c/i2c.h"
#include "esphome/components/switch/switch.h"
#include "GasBreakout.h"

namespace esphome {
namespace mics6814 {

class MICS6814Component : public PollingComponent, public i2c::I2CDevice {
 public:
  MICS6814Component(uint32_t update_interval = 60000) : PollingComponent(update_interval) {}

  void setup() override;
  void update() override;
  void dump_config() override;
  float get_setup_priority() const override { return esphome::setup_priority::BUS; }

  // Sensor setters
  void set_reducing_resistance_sensor(sensor::Sensor *sensor) { reducing_resistance_sensor_ = sensor; }
  void set_nh3_resistance_sensor(sensor::Sensor *sensor) { nh3_resistance_sensor_ = sensor; }
  void set_oxidising_resistance_sensor(sensor::Sensor *sensor) { oxidising_resistance_sensor_ = sensor; }

  void set_co_sensor(sensor::Sensor *sensor) { co_sensor_ = sensor; }
  void set_no2_sensor(sensor::Sensor *sensor) { no2_sensor_ = sensor; }
  void set_nh3_sensor(sensor::Sensor *sensor) { nh3_sensor_ = sensor; }
  void set_c3h8_sensor(sensor::Sensor *sensor) { c3h8_sensor_ = sensor; }
  void set_c4h10_sensor(sensor::Sensor *sensor) { c4h10_sensor_ = sensor; }
  void set_ch4_sensor(sensor::Sensor *sensor) { ch4_sensor_ = sensor; }
  void set_h2_sensor(sensor::Sensor *sensor) { h2_sensor_ = sensor; }
  void set_c2h5oh_sensor(sensor::Sensor *sensor) { c2h5oh_sensor_ = sensor; }

  // Switch setters
  void set_calibration_switch(switch_::Switch *sw) { calibration_switch_ = sw; }
  void set_heater_switch(switch_::Switch *sw) { heater_switch_ = sw; }

  // For internal use by switch classes
  void set_heater(bool state);
  void set_calibration(bool state);
  void set_led_r(uint8_t value);
  void set_led_g(uint8_t value);
  void set_led_b(uint8_t value);

  // Override to sync update_interval_seconds_
  void set_update_interval(uint32_t interval) override;

 protected:
  bool read_resistances(float &reducing, float &nh3, float &oxidising);
  void calculate_gas_concentrations(float reducing, float nh3, float oxidising);
  void update_resistance_sensors(float reducing, float nh3, float oxidising);

  // Reference resistances for calibration
  float reducing_ref_{0};
  float nh3_ref_{0};
  float oxidising_ref_{0};

  // Sensor pointers
  sensor::Sensor *reducing_resistance_sensor_{nullptr};
  sensor::Sensor *nh3_resistance_sensor_{nullptr};
  sensor::Sensor *oxidising_resistance_sensor_{nullptr};

  sensor::Sensor *co_sensor_{nullptr};
  sensor::Sensor *no2_sensor_{nullptr};
  sensor::Sensor *nh3_sensor_{nullptr};
  sensor::Sensor *c3h8_sensor_{nullptr};
  sensor::Sensor *c4h10_sensor_{nullptr};
  sensor::Sensor *ch4_sensor_{nullptr};
  sensor::Sensor *h2_sensor_{nullptr};
  sensor::Sensor *c2h5oh_sensor_{nullptr};

  switch_::Switch *calibration_switch_{nullptr};
  switch_::Switch *heater_switch_{nullptr};

  GasBreakout *gas_{nullptr};
  bool sensor_present_{false};
  uint32_t update_interval_ms_{60000};
  bool prev_heater_state_{false};
  uint32_t last_reading_ms_{0};
  uint32_t heater_start_ms_{0};
  bool heater_warmup_{false};
  bool heater_manual_override_{false};
};

// Switch for heater control
class MICS6814HeaterSwitch : public switch_::Switch, public Component {
 public:
  explicit MICS6814HeaterSwitch(MICS6814Component *parent) : parent_(parent) {}

  void set_parent(MICS6814Component *parent) { parent_ = parent; }

  void write_state(bool state) override {
    if (parent_ != nullptr) {
      parent_->set_heater(state);
    }
    publish_state(state);
  }

 protected:
  MICS6814Component *parent_;
};

// Switch for calibration
class MICS6814CalibrationSwitch : public switch_::Switch, public Component {
 public:
  explicit MICS6814CalibrationSwitch(MICS6814Component *parent) : parent_(parent) {}

  void set_parent(MICS6814Component *parent) { parent_ = parent; }

  void write_state(bool state) override {
    if (parent_ != nullptr) {
      parent_->set_calibration(state);
    }
    publish_state(state);
  }

 protected:
  MICS6814Component *parent_;
};

}  // namespace mics6814
}  // namespace esphome
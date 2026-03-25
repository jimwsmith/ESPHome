#include "mics6814_component.h"
#include "esphome/components/preferences/preferences.h"

namespace esphome {
namespace mics6814 {

static const char *const TAG = "mics6814";

void MICS6814Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MiCS-6814...");

  // Create GasBreakout instance using I2CDevice's wire
  if (this->get_wire() == nullptr) {
    ESP_LOGE(TAG, "I2C wire not available!");
    this->mark_failed();
    return;
  }
  gas_ = new GasBreakout(*this->get_wire(), this->get_address());

  if (!gas_->initialise()) {
    ESP_LOGE(TAG, "Failed to initialize MiCS-6814 sensor!");
    sensor_present_ = false;
    this->mark_failed();
    return;
  }

  ESP_LOGD(TAG, "MiCS-6814 initialized successfully");
  sensor_present_ = true;

  // Initialize with heater off
  gas_->setHeater(false);
  gas_->setR(0);
  gas_->setG(0);
  gas_->setB(0);

  // Load calibration from preferences if available
  auto *prefs = preferences::global_preferences;
  size_t offset = 0;
  if (prefs->load(offset, &reducing_ref_, sizeof(reducing_ref_))) {
    offset += sizeof(reducing_ref_);
    if (prefs->load(offset, &nh3_ref_, sizeof(nh3_ref_))) {
      offset += sizeof(nh3_ref_);
      if (prefs->load(offset, &oxidising_ref_, sizeof(oxidising_ref_))) {
        ESP_LOGD(TAG, "Loaded calibration: R=%.0f, NH3=%.0f, OX=%.0f",
                 reducing_ref_, nh3_ref_, oxidising_ref_);
      }
    }
  }
}

void MICS6814Component::update() {
  if (!sensor_present_ || gas_ == nullptr) {
    return;
  }

  uint32_t now = millis();
  static const uint32_t HEATER_WARMUP_MS = 4000;

  // Check if it's time for a reading
  if (last_reading_ms_ == 0) {
    // First reading, initialize timer
    last_reading_ms_ = now;
    return;
  }

  uint32_t time_since_last_reading = now - last_reading_ms_;

  // Check if heater should be turned on for warmup (4 seconds before reading)
  if (!heater_warmup_ && time_since_last_reading >= (update_interval_ms_ - HEATER_WARMUP_MS)) {
    // Start heater warmup, but only if not manually overridden
    if (heater_switch_ == nullptr || !heater_switch_->state) {
      gas_->setHeater(true);
      gas_->setR(64); // Red LED on for heater
      heater_warmup_ = true;
      heater_start_ms_ = now;
      ESP_LOGD(TAG, "Heater warmup started");
    }
  }

  // Check if warmup complete and time to take reading
  if (heater_warmup_ && (now - heater_start_ms_ >= HEATER_WARMUP_MS)) {
    // Take measurement
    GasBreakout::Reading reading = gas_->readAll();
    float red_val = reading.reducing;
    float nh3_val = reading.nh3;
    float ox_val = reading.oxidising;

    // Restore heater state based on switch
    if (heater_switch_ != nullptr) {
      gas_->setHeater(heater_switch_->state);
      if (!heater_switch_->state) {
        gas_->setR(0);
      }
    } else {
      gas_->setHeater(false);
      gas_->setR(0);
    }

    // Update resistance sensors
    update_resistance_sensors(red_val, nh3_val, ox_val);

    // Handle calibration
    if (calibration_switch_ != nullptr && calibration_switch_->state) {
      reducing_ref_ = red_val;
      nh3_ref_ = nh3_val;
      oxidising_ref_ = ox_val;
      ESP_LOGD(TAG, "Calibration values saved: R=%.0f, NH3=%.0f, OX=%.0f",
               reducing_ref_, nh3_ref_, oxidising_ref_);

      // Save to preferences
      auto *prefs = preferences::global_preferences;
      size_t offset = 0;
      prefs->save(offset, &reducing_ref_, sizeof(reducing_ref_));
      offset += sizeof(reducing_ref_);
      prefs->save(offset, &nh3_ref_, sizeof(nh3_ref_));
      offset += sizeof(nh3_ref_);
      prefs->save(offset, &oxidising_ref_, sizeof(oxidising_ref_));
      ESP_LOGD(TAG, "Calibration saved to preferences");

      // Turn off calibration switch
      calibration_switch_->turn_off();
    }

    // Calculate gas concentrations if we have reference values
    if (reducing_ref_ > 0 && nh3_ref_ > 0 && oxidising_ref_ > 0) {
      calculate_gas_concentrations(red_val, nh3_val, ox_val);
    }

    // Reset for next cycle
    last_reading_ms_ = now;
    heater_warmup_ = false;
    ESP_LOGD(TAG, "Reading taken, next in %u ms", update_interval_ms_);
  }
}

void MICS6814Component::dump_config() {
  ESP_LOGCONFIG(TAG, "MiCS-6814:");
  LOG_I2C_DEVICE(this);
  if (this->is_failed()) {
    ESP_LOGE(TAG, "Connection failed!");
  }
  ESP_LOGCONFIG(TAG, "  Update Interval: %u seconds", update_interval_ms_ / 1000);
  ESP_LOGCONFIG(TAG, "  Calibration Reference - Reducing: %.0f ohm", reducing_ref_);
  ESP_LOGCONFIG(TAG, "  Calibration Reference - NH3: %.0f ohm", nh3_ref_);
  ESP_LOGCONFIG(TAG, "  Calibration Reference - Oxidising: %.0f ohm", oxidising_ref_);
}

void MICS6814Component::set_heater(bool state) {
  if (gas_ != nullptr) {
    gas_->setHeater(state);
    if (state) {
      gas_->setR(64);
    } else {
      gas_->setR(0);
    }
  }
  prev_heater_state_ = state;

  // If user manually controls heater, cancel any active warmup
  heater_warmup_ = false;
}

void MICS6814Component::set_calibration(bool state) {
  if (state && gas_ != nullptr) {
    gas_->setG(32); // Green LED on during calibration
  } else if (gas_ != nullptr) {
    gas_->setG(0); // Green LED off
  }
}

void MICS6814Component::set_led_r(uint8_t value) {
  if (gas_ != nullptr) {
    gas_->setR(value);
  }
}

void MICS6814Component::set_led_g(uint8_t value) {
  if (gas_ != nullptr) {
    gas_->setG(value);
  }
}

void MICS6814Component::set_led_b(uint8_t value) {
  if (gas_ != nullptr) {
    gas_->setB(value);
  }
}

void MICS6814Component::update_resistance_sensors(float reducing, float nh3, float oxidising) {
  if (reducing_resistance_sensor_ != nullptr) {
    reducing_resistance_sensor_->publish_state(reducing);
  }
  if (nh3_resistance_sensor_ != nullptr) {
    nh3_resistance_sensor_->publish_state(nh3);
  }
  if (oxidising_resistance_sensor_ != nullptr) {
    oxidising_resistance_sensor_->publish_state(oxidising);
  }
}

void MICS6814Component::calculate_gas_concentrations(float reducing, float nh3, float oxidising) {
  // Calculate ratios
  float red_ratio = reducing / reducing_ref_;
  float nh3_ratio = nh3 / nh3_ref_;
  float ox_ratio = oxidising / oxidising_ref_;

  ESP_LOGD(TAG, "Resistance ratios - Red: %.3f, NH3: %.3f, OX: %.3f",
           red_ratio, nh3_ratio, ox_ratio);

  // Calculate gas concentrations using formulas from datasheet
  // Note: These formulas are approximate and may need calibration

  // Carbon monoxide (CO) in ppm
  if (co_sensor_ != nullptr) {
    float co_ppm = (pow(red_ratio, -1.177f) - 1.0f) * 4.4638f;
    co_sensor_->publish_state(co_ppm);
  }

  // Nitrogen dioxide (NO2) in ppm
  if (no2_sensor_ != nullptr) {
    float no2_ppm = (pow(ox_ratio, 0.9979f) - 1.0f) * 0.1516f;
    no2_sensor_->publish_state(no2_ppm);
  }

  // Ammonia (NH3) in µg/m³
  if (nh3_sensor_ != nullptr) {
    float nh3_ugm3 = (pow(nh3_ratio, -1.903f) - 1.0f) * 0.6151f;
    nh3_sensor_->publish_state(nh3_ugm3);
  }

  // Propane (C3H8) in ppm
  if (c3h8_sensor_ != nullptr) {
    float c3h8_ppm = (pow(nh3_ratio, -2.492f) - 1.0f) * 569.56f;
    c3h8_sensor_->publish_state(c3h8_ppm);
  }

  // Iso-butane (C4H10) in ppm
  if (c4h10_sensor_ != nullptr) {
    float c4h10_ppm = (pow(nh3_ratio, -1.888f) - 1.0f) * 503.2f;
    c4h10_sensor_->publish_state(c4h10_ppm);
  }

  // Methane (CH4) in ppm
  if (ch4_sensor_ != nullptr) {
    float ch4_ppm = (pow(red_ratio, -4.093f) - 1.0f) * 837.38f;
    ch4_sensor_->publish_state(ch4_ppm);
  }

  // Hydrogen (H2) in ppm
  if (h2_sensor_ != nullptr) {
    float h2_ppm = (pow(red_ratio, -1.781f) - 1.0f) * 0.828f;
    h2_sensor_->publish_state(h2_ppm);
  }

  // Ethanol (C2H5OH) in ppm
  if (c2h5oh_sensor_ != nullptr) {
    float c2h5oh_ppm = (pow(red_ratio, -1.58f) - 1.0f) * 1.363f;
    c2h5oh_sensor_->publish_state(c2h5oh_ppm);
  }
}

bool MICS6814Component::read_resistances(float &reducing, float &nh3, float &oxidising) {
  if (!sensor_present_ || gas_ == nullptr) {
    return false;
  }
  GasBreakout::Reading reading = gas_->readAll();
  reducing = reading.reducing;
  nh3 = reading.nh3;
  oxidising = reading.oxidising;
  return true;
}

void MICS6814Component::set_update_interval(uint32_t interval) {
  // Enforce minimum interval of 5 seconds (5000ms) for heater warmup
  if (interval < 5000) {
    ESP_LOGW(TAG, "Update interval too short (%u ms), minimum is 5000 ms", interval);
    interval = 5000;
  }
  PollingComponent::set_update_interval(interval);
  update_interval_ms_ = interval;
}

}  // namespace mics6814
}  // namespace esphome
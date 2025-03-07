#include "esphome.h"
#include "GasBreakout.h"

using namespace esphome;

/***** Global Constants *****/
static const unsigned int MICS6814_DELAY_MS = 1 * 1000; //loop_interval
static const unsigned int MISC6814_UPDATE_INTERVAL = 60; //Loop count in seconds before reading taken and sensor updated

/***** Global Variables *****/
GasBreakout gas(Wire, 0x19);

class mics6814 : public PollingComponent {
  public:
    Sensor *redres_sensor = new Sensor();
    Sensor *nh3res_sensor = new Sensor();
    Sensor *oxres_sensor = new Sensor();
    Sensor *co_sensor = new Sensor();
    Sensor *no2_sensor = new Sensor();
    Sensor *nh3_sensor = new Sensor();
    Sensor *c3h8_sensor = new Sensor();
    Sensor *c4h10_sensor = new Sensor();
    Sensor *ch4_sensor = new Sensor();
    Sensor *h2_sensor = new Sensor();
    Sensor *c2h5oh_sensor = new Sensor();

    mics6814() : PollingComponent(MICS6814_DELAY_MS) { }  //Constructor defines polling interval

    float get_setup_priority() const override { return esphome::setup_priority::BUS; }

    bool mics6814_present = false;

    void setup() override {

      Wire.begin();

      if(!gas.initialise()){
        ESP_LOGD("MCS6814","Not Initialised");
      } else {
        ESP_LOGD("MCS6814", "Initialised");
        mics6814_present = true;
      }
    }

  void update() override {
    static float redprev, nh3prev, oxprev; //Retain calibration resistances
    static int polling_count; 

    GasBreakout::Reading reading; //Declare result structure
    float redval, nh3val, oxval;
    float redratio, nh3ratio, oxratio;
    bool prev_heater = id(heater_override).state;

    if (mics6814_present) {
      polling_count +=1; //Increment Polling count, once every second
//            ESP_LOGD("MCS6814", "Loop %d", polling_count); //Debug log
      if (polling_count > MISC6814_UPDATE_INTERVAL-4){ //Seconds loop count to enable heater
        gas.setHeater(true); //Heater On
        gas.setR(64);
        if (polling_count > MISC6814_UPDATE_INTERVAL){ //Seconds loop count to take measurement after it stabilises
          polling_count = 0; //Reset counter
          reading = gas.readAll();
          redval = reading.reducing;
          nh3val = reading.nh3;
          oxval = reading.oxidising;
          gas.setHeater(prev_heater); //Heater restore to previous state
          if (!prev_heater) gas.setR(0); //Turn off Red LED if switch was previously set to off

          redres_sensor->publish_state(reading.reducing);
          nh3res_sensor->publish_state(reading.nh3);
          oxres_sensor->publish_state(reading.oxidising);

          if (id(calibration_switch).state){ //We want to save these readings as Reference Resistance values to calculate gas % later
            redprev = redval;
            nh3prev = nh3val;
            oxprev = oxval;
            ESP_LOGD("MCS6814","saved calib"); //Debug log
          }
          if (redprev!=0 && nh3prev!=0 && oxprev!=0){
            // Calculate ratios of resistances
            redratio = redval / redprev;
            nh3ratio = nh3val / nh3prev;
            oxratio = oxval/ oxprev;
            ESP_LOGD("MCS6814","Calib ratio Red %f, NH3 %f, OX %f", redratio, nh3ratio, oxratio); //Debug log

            //Equations corrected to give zero as output for resistance ratio of 1
            //carbon monoxide
            co_sensor->publish_state((pow(redratio, -1.177)-1) * 4.4638);
            //nitrogen dioxide
            no2_sensor->publish_state((pow(oxratio, 0.9979)-1) * 0.1516);
            //ammonia
            nh3_sensor->publish_state((pow(nh3ratio, -1.903)-1) * 0.6151);
            //propane
            c3h8_sensor->publish_state((pow(nh3ratio, -2.492)-1) * 569.56);
            //iso-butane
            c4h10_sensor->publish_state((pow(nh3ratio, -1.888)-1) * 503.2);
            //methane
            ch4_sensor->publish_state((pow(redratio, -4.093)-1) * 837.38);
            //hydrogen
            h2_sensor->publish_state((pow(redratio, -1.781)-1) * 0.828);
            //ethanol
            c2h5oh_sensor->publish_state((pow(redratio, -1.58)-1) * 1.363);

            if (id(calibration_switch).state) id(calibration_switch).turn_off(); //Turn off calibration when done
          }
        }
      }
    }
  }
};

class HeaterSwitch : public Component, public Switch {
  public:

    float get_setup_priority() const override { return esphome::setup_priority::LATE; }

    void setup() override {
//      publish_state(false);
    }

    void write_state(bool state) override {
      gas.setHeater(state);
      publish_state(state);
      if (state) gas.setR(64); else gas.setR(0);
    }
};

class CalibrationSwitch : public Component, public Switch {
  public:

    float get_setup_priority() const override { return esphome::setup_priority::LATE; }

    void setup() override {
//      publish_state(false);  //Force state to off at powerup
    }

    void write_state(bool state) override {
      publish_state(state);

      if (state){
        gas.setG(32); //Green LED on
      } else {
        gas.setG(0); //Green LED off
      }
    }
};

class LedChannel : public Component, public FloatOutput {
  public:
    float get_setup_priority() const override { return esphome::setup_priority::LATE; }
    void (GasBreakout::*ch_handler)(uint8_t v);

    LedChannel(char chan) {
      switch(chan)
      {
        case 'R': ch_handler=&GasBreakout::setR; break;
        case 'G': ch_handler=&GasBreakout::setG; break;
        case 'B': ch_handler=&GasBreakout::setB; break;
        default:
          ch_handler=NULL;
          ESP_LOGD("custom", "no callback");
      }
    }

    void write_state(float state) override {
      uint8_t period = state * 255;
      if (ch_handler != NULL)
        (&gas->*ch_handler)(period);
    }
};

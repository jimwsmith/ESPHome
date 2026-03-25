import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor, i2c
from esphome.const import (
    CONF_ID,
    UNIT_OHM,
    UNIT_PARTS_PER_MILLION,
    UNIT_MICROGRAMS_PER_CUBIC_METER,
    DEVICE_CLASS_AQI,
    STATE_CLASS_MEASUREMENT,
    ENTITY_CATEGORY_DIAGNOSTIC,
    ICON_GAS_CYLINDER,
    ICON_GAUGE,
)

from .. import mics6814_ns, CONF_MICS6814_ID, MICS6814Component, to_code as mics6814_to_code

DEPENDENCIES = ["mics6814"]

# Resistance sensors
CONF_REDUCING_RESISTANCE = "reducing_resistance"
CONF_NH3_RESISTANCE = "nh3_resistance"
CONF_OXIDISING_RESISTANCE = "oxidising_resistance"

# Gas sensors
CONF_CO = "carbon_monoxide"
CONF_NO2 = "nitrogen_dioxide"
CONF_NH3 = "ammonia"
CONF_C3H8 = "propane"
CONF_C4H10 = "iso_butane"
CONF_CH4 = "methane"
CONF_H2 = "hydrogen"
CONF_C2H5OH = "ethanol"

# MICS6814Sensor removed, using MICS6814Component

BASE_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_MICS6814_ID): cv.use_id(MICS6814Component),
})

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.Optional(CONF_REDUCING_RESISTANCE): sensor.sensor_schema(
                unit_of_measurement=UNIT_OHM,
                icon=ICON_GAUGE,
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_NH3_RESISTANCE): sensor.sensor_schema(
                unit_of_measurement=UNIT_OHM,
                icon=ICON_GAUGE,
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_OXIDISING_RESISTANCE): sensor.sensor_schema(
                unit_of_measurement=UNIT_OHM,
                icon=ICON_GAUGE,
                accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CO): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_GAS_CYLINDER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_NO2): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_GAS_CYLINDER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_NH3): sensor.sensor_schema(
                unit_of_measurement=UNIT_MICROGRAMS_PER_CUBIC_METER,
                icon=ICON_GAS_CYLINDER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_C3H8): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_GAS_CYLINDER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_C4H10): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_GAS_CYLINDER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_CH4): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_GAS_CYLINDER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_H2): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_GAS_CYLINDER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
            cv.Optional(CONF_C2H5OH): sensor.sensor_schema(
                unit_of_measurement=UNIT_PARTS_PER_MILLION,
                icon=ICON_GAS_CYLINDER,
                accuracy_decimals=0,
                state_class=STATE_CLASS_MEASUREMENT,
            ),
        }
    )
    .extend(BASE_SCHEMA)
)

async def to_code(config):
    await mics6814_to_code(config)
    parent = await cg.get_variable(config[CONF_MICS6814_ID])

    if CONF_REDUCING_RESISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_REDUCING_RESISTANCE])
        cg.add(parent.set_reducing_resistance_sensor(sens))

    if CONF_NH3_RESISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_NH3_RESISTANCE])
        cg.add(parent.set_nh3_resistance_sensor(sens))

    if CONF_OXIDISING_RESISTANCE in config:
        sens = await sensor.new_sensor(config[CONF_OXIDISING_RESISTANCE])
        cg.add(parent.set_oxidising_resistance_sensor(sens))

    if CONF_CO in config:
        sens = await sensor.new_sensor(config[CONF_CO])
        cg.add(parent.set_co_sensor(sens))

    if CONF_NO2 in config:
        sens = await sensor.new_sensor(config[CONF_NO2])
        cg.add(parent.set_no2_sensor(sens))

    if CONF_NH3 in config:
        sens = await sensor.new_sensor(config[CONF_NH3])
        cg.add(parent.set_nh3_sensor(sens))

    if CONF_C3H8 in config:
        sens = await sensor.new_sensor(config[CONF_C3H8])
        cg.add(parent.set_c3h8_sensor(sens))

    if CONF_C4H10 in config:
        sens = await sensor.new_sensor(config[CONF_C4H10])
        cg.add(parent.set_c4h10_sensor(sens))

    if CONF_CH4 in config:
        sens = await sensor.new_sensor(config[CONF_CH4])
        cg.add(parent.set_ch4_sensor(sens))

    if CONF_H2 in config:
        sens = await sensor.new_sensor(config[CONF_H2])
        cg.add(parent.set_h2_sensor(sens))

    if CONF_C2H5OH in config:
        sens = await sensor.new_sensor(config[CONF_C2H5OH])
        cg.add(parent.set_c2h5oh_sensor(sens))
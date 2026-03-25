import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID, CONF_UPDATE_INTERVAL

DEPENDENCIES = ["i2c"]
CODEOWNERS = ["@user"]

CONF_MICS6814_ID = "mics6814_id"

mics6814_ns = cg.esphome_ns.namespace("mics6814")
MICS6814Component = mics6814_ns.class_("MICS6814Component", cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(MICS6814Component),
            cv.Optional(CONF_UPDATE_INTERVAL, default="60s"): cv.update_interval,
        }
    )
    .extend(cv.polling_component_schema("60s"))
    .extend(i2c.i2c_device_schema(0x19))
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL].total_milliseconds))

    # Add dependency on GasBreakout library (part of IOExpander_Library)
    cg.add_library("ZodiusInfuser/IOExpander_Library")
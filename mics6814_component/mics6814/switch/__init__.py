import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import switch
from esphome.const import CONF_ID, CONF_ICON, ENTITY_CATEGORY_CONFIG

from .. import mics6814_ns, CONF_MICS6814_ID

DEPENDENCIES = ["mics6814"]

CONF_HEATER = "heater"
CONF_CALIBRATION = "calibration"

MICS6814HeaterSwitch = mics6814_ns.class_("MICS6814HeaterSwitch", switch.Switch, cg.Component)
MICS6814CalibrationSwitch = mics6814_ns.class_("MICS6814CalibrationSwitch", switch.Switch, cg.Component)

BASE_SCHEMA = cv.Schema({
    cv.GenerateID(CONF_MICS6814_ID): cv.use_id(mics6814_ns.MICS6814Component),
})

CONFIG_SCHEMA = cv.Schema(
    {
        cv.Optional(CONF_HEATER): switch.switch_schema(
            MICS6814HeaterSwitch,
            icon="mdi:fire",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_CALIBRATION): switch.switch_schema(
            MICS6814CalibrationSwitch,
            icon="mdi:adjust",
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
    }
).extend(BASE_SCHEMA)

async def to_code(config):
    parent = await cg.get_variable(config[CONF_MICS6814_ID])

    if CONF_HEATER in config:
        conf = config[CONF_HEATER]
        sw = cg.new_Pvariable(conf[CONF_ID])
        await switch.register_switch(sw, conf)
        await cg.register_component(sw, conf)
        cg.add(sw.set_parent(parent))

    if CONF_CALIBRATION in config:
        conf = config[CONF_CALIBRATION]
        sw = cg.new_Pvariable(conf[CONF_ID])
        await switch.register_switch(sw, conf)
        await cg.register_component(sw, conf)
        cg.add(sw.set_parent(parent))
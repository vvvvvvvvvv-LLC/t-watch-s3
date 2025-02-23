//
// Copyright (c) 2025 Noah Luskey <noah@vvvvvvvvvv.io>
// SPDX-License-Identifier: Apache-2.0
//

#define DT_DRV_COMPAT x_powers_axp2101

#include <errno.h>
#include <stdbool.h>

#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(axp2101, CONFIG_AXP2101_LOG_LEVEL);

// Chip ID value and register
#define AXP2101_CHIP_ID 0x4A
#define AXP2101_REG_CHIP_ID 0x03U

// Register and bit to enable button battery charging (for RTC)
#define AXP2101_CHARGER_FUEL_GAUGE_WATCHDOG_CTRL_REG 0x18U
#define AXP2101_CHARGER_FUEL_GAUGE_WATCHDOG_CTRL_BUTTON_CHARGER_MASK BIT(2)

struct axp2101_config
{
        struct i2c_dt_spec i2c;
        bool button_battery_charge_enable;
};

static int axp2101_init(const struct device *dev)
{
        const struct axp2101_config *config = dev->config;
        uint8_t chip_id;
        int ret;

        LOG_DBG("Initializing instance");

        if (!i2c_is_ready_dt(&config->i2c))
        {
                LOG_ERR("I2C bus not ready");
                return -ENODEV;
        }

        // Check if axp2101 chip is available
        ret = i2c_reg_read_byte_dt(&config->i2c, AXP2101_REG_CHIP_ID, &chip_id);
        if (ret < 0)
        {
                return ret;
        }

        if (chip_id != AXP2101_CHIP_ID)
        {
                LOG_ERR("Invalid Chip detected (%d)", chip_id);
                return -EINVAL;
        }

        // enable coin battery charging through VBACKUP if requested
        if (config->button_battery_charge_enable)
        {
                ret = i2c_reg_update_byte_dt(&config->i2c, AXP2101_CHARGER_FUEL_GAUGE_WATCHDOG_CTRL_REG, BIT(2), BIT(2));
                if (ret < 0)
                {
                        LOG_ERR("Could not enable coin battery charging (%d)", ret);
                        return ret;
                }
        }

        return 0;
}

// I2C must be initialized before the AXP2101 driver
BUILD_ASSERT(CONFIG_AXP2101_INIT_PRIORITY > CONFIG_I2C_INIT_PRIORITY);

#define AXP2101_DEFINE(inst)                                                                   \
        static const struct axp2101_config config##inst = {                                    \
            .i2c = I2C_DT_SPEC_INST_GET(inst),                                                 \
            .button_battery_charge_enable = DT_INST_PROP(inst, button_battery_charge_enable)}; \
                                                                                               \
        DEVICE_DT_INST_DEFINE(inst, axp2101_init, NULL, NULL, &config##inst, POST_KERNEL,      \
                              CONFIG_AXP2101_INIT_PRIORITY, NULL);

DT_INST_FOREACH_STATUS_OKAY(AXP2101_DEFINE)

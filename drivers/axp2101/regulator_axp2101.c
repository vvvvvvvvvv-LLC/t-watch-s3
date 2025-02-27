//  Copyright (c) 2021 NXP
//  Copyright (c) 2023 Martin Kiepfer <mrmarteng@teleschirm.org>
//  Copyright (c) 2024 Noah Luskey <noah@vvvvvvvvvv.io>
//  SPDX-License-Identifier: Apache-2.0

#define DT_DRV_COMPAT x_powers_axp2101_regulator

#include <errno.h>

#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/regulator.h>
#include <zephyr/sys/linear_range.h>
#include <zephyr/sys/util.h>
#include <zephyr/dt-bindings/regulator/axp2101.h>
#include <zephyr/logging/log.h>
#include <zephyr/logging/log_instance.h>

LOG_MODULE_REGISTER(regulator_axp2101, CONFIG_AXP2101_LOG_LEVEL);

// Output control registers
#define AXP2101_REG_DCDC12345_CONTROL 0x80
#define AXP2101_REG_DCDC1234_WORKMODE 0x81
#define AXP2101_REG_DCDC1_VOLTAGE 0x82
#define AXP2101_REG_DCDC2_VOLTAGE 0x83
#define AXP2101_REG_DCDC3_VOLTAGE 0x84
#define AXP2101_REG_DCDC4_VOLTAGE 0x85
#define AXP2101_REG_DCDC5_VOLTAGE 0x86
#define AXP2101_REG_ALDO1234_BLDO12_CPUSLDO_DLDO1_CONTROL 0x90
#define AXP2101_REG_DLDO2_CONTROL 0x91
#define AXP2101_REG_ALDO1_VOLTAGE 0x92
#define AXP2101_REG_ALDO2_VOLTAGE 0x93
#define AXP2101_REG_ALDO3_VOLTAGE 0x94
#define AXP2101_REG_ALDO4_VOLTAGE 0x95
#define AXP2101_REG_BLDO1_VOLTAGE 0x96
#define AXP2101_REG_BLDO2_VOLTAGE 0x97
#define AXP2101_REG_CPUSLDO_VOLTAGE 0x98
#define AXP2101_REG_DLDO1_VOLTAGE 0x99
#define AXP2101_REG_DLDO2_VOLTAGE 0x9A

struct regulator_axp2101_desc
{
	const uint8_t enable_reg;
	const uint8_t enable_mask;
	const uint8_t enable_val;
	const uint8_t vsel_reg;
	const uint8_t vsel_mask;
	const uint8_t vsel_bitpos;
	const int32_t max_ua;
	const uint8_t workmode_reg;
	const uint8_t workmode_mask;
	const uint8_t workmode_pwm_val;
	const uint8_t num_ranges;
	const struct linear_range *ranges;
};

struct regulator_axp2101_data
{
	struct regulator_common_data data;
};

struct regulator_axp2101_config
{
	struct regulator_common_config common;
	const struct regulator_axp2101_desc *desc;
	const struct device *mfd;
	const struct i2c_dt_spec i2c;

	LOG_INSTANCE_PTR_DECLARE(log);
};

__maybe_unused static const struct linear_range dcdc1_ranges[] = {
	LINEAR_RANGE_INIT(1500000, 100000U, 0x00U, 0x13U),
};

__maybe_unused static const struct regulator_axp2101_desc dcdc1_desc = {
	.enable_reg = AXP2101_REG_DCDC12345_CONTROL,
	.enable_mask = 0x01U,
	.enable_val = 0x01U,
	.vsel_reg = AXP2101_REG_DCDC1_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 2000000U,
	.workmode_reg = AXP2101_REG_DCDC1234_WORKMODE,
	.workmode_mask = 0x04U,
	.workmode_pwm_val = 0x04U,
	.ranges = dcdc1_ranges,
	.num_ranges = ARRAY_SIZE(dcdc1_ranges),
};

__maybe_unused static const struct linear_range dcdc2_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 10000U, 0x00U, 0x46U),
	LINEAR_RANGE_INIT(1220000U, 20000U, 0x47U, 0x57U),
};

__maybe_unused static const struct regulator_axp2101_desc dcdc2_desc = {
	.enable_reg = AXP2101_REG_DCDC12345_CONTROL,
	.enable_mask = 0x02U,
	.enable_val = 0x02U,
	.vsel_reg = AXP2101_REG_DCDC2_VOLTAGE,
	.vsel_mask = 0x7FU,
	.vsel_bitpos = 0U,
	.max_ua = 2000000U,
	.workmode_reg = AXP2101_REG_DCDC1234_WORKMODE,
	.workmode_mask = 0x08U,
	.workmode_pwm_val = 0x08U,
	.ranges = dcdc2_ranges,
	.num_ranges = ARRAY_SIZE(dcdc2_ranges),
};

__maybe_unused static const struct linear_range dcdc3_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 10000U, 0x00U, 0x46U),
	LINEAR_RANGE_INIT(1220000U, 20000U, 0x47U, 0x57U),
	LINEAR_RANGE_INIT(1600000U, 100000U, 0x58U, 0x6BU),
};

__maybe_unused static const struct regulator_axp2101_desc dcdc3_desc = {
	.enable_reg = AXP2101_REG_DCDC12345_CONTROL,
	.enable_mask = 0x04U,
	.enable_val = 0x04U,
	.vsel_reg = AXP2101_REG_DCDC3_VOLTAGE,
	.vsel_mask = 0x7FU,
	.vsel_bitpos = 0U,
	.max_ua = 2000000U,
	.workmode_reg = AXP2101_REG_DCDC1234_WORKMODE,
	.workmode_mask = 0x10U,
	.workmode_pwm_val = 0x10U,
	.ranges = dcdc3_ranges,
	.num_ranges = ARRAY_SIZE(dcdc3_ranges),
};

__maybe_unused static const struct linear_range dcdc4_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 10000U, 0x00U, 0x46U),
	LINEAR_RANGE_INIT(1220000U, 20000U, 0x47U, 0x66U),
};

__maybe_unused static const struct regulator_axp2101_desc dcdc4_desc = {
	.enable_reg = AXP2101_REG_DCDC12345_CONTROL,
	.enable_mask = 0x08U,
	.enable_val = 0x08U,
	.vsel_reg = AXP2101_REG_DCDC4_VOLTAGE,
	.vsel_mask = 0x7FU,
	.vsel_bitpos = 0U,
	.max_ua = 1500000U,
	.workmode_reg = AXP2101_REG_DCDC1234_WORKMODE,
	.workmode_mask = 0x20U,
	.workmode_pwm_val = 0x20U,
	.ranges = dcdc4_ranges,
	.num_ranges = ARRAY_SIZE(dcdc4_ranges),
};

__maybe_unused static const struct linear_range dcdc5_ranges[] = {
	LINEAR_RANGE_INIT(1400000U, 100000U, 0x00U, 0x17U),
};

__maybe_unused static const struct regulator_axp2101_desc dcdc5_desc = {
	.enable_reg = AXP2101_REG_DCDC12345_CONTROL,
	.enable_mask = 0x10U,
	.enable_val = 0x10U,
	.vsel_reg = AXP2101_REG_DCDC5_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 1000000U,
	.ranges = dcdc5_ranges,
	.num_ranges = ARRAY_SIZE(dcdc5_ranges),
};

__maybe_unused static const struct linear_range aldo1_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 100000U, 0x00U, 0x1EU),
};

__maybe_unused static const struct regulator_axp2101_desc aldo1_desc = {
	.enable_reg = AXP2101_REG_ALDO1234_BLDO12_CPUSLDO_DLDO1_CONTROL,
	.enable_mask = 0x01U,
	.enable_val = 0x01U,
	.vsel_reg = AXP2101_REG_ALDO1_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 300000U,
	.ranges = aldo1_ranges,
	.num_ranges = ARRAY_SIZE(aldo1_ranges),
};

__maybe_unused static const struct linear_range aldo2_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 100000U, 0x00U, 0x1EU),
};

__maybe_unused static const struct regulator_axp2101_desc aldo2_desc = {
	.enable_reg = AXP2101_REG_ALDO1234_BLDO12_CPUSLDO_DLDO1_CONTROL,
	.enable_mask = 0x02U,
	.enable_val = 0x02U,
	.vsel_reg = AXP2101_REG_ALDO2_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 300000U,
	.ranges = aldo2_ranges,
	.num_ranges = ARRAY_SIZE(aldo2_ranges),
};

__maybe_unused static const struct linear_range aldo3_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 100000U, 0x00U, 0x1EU),
};

__maybe_unused static const struct regulator_axp2101_desc aldo3_desc = {
	.enable_reg = AXP2101_REG_ALDO1234_BLDO12_CPUSLDO_DLDO1_CONTROL,
	.enable_mask = 0x04U,
	.enable_val = 0x04U,
	.vsel_reg = AXP2101_REG_ALDO3_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 300000U,
	.ranges = aldo3_ranges,
	.num_ranges = ARRAY_SIZE(aldo3_ranges),
};

__maybe_unused static const struct linear_range aldo4_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 100000U, 0x00U, 0x1EU),
};

__maybe_unused static const struct regulator_axp2101_desc aldo4_desc = {
	.enable_reg = AXP2101_REG_ALDO1234_BLDO12_CPUSLDO_DLDO1_CONTROL,
	.enable_mask = 0x08U,
	.enable_val = 0x08U,
	.vsel_reg = AXP2101_REG_ALDO4_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 300000U,
	.ranges = aldo4_ranges,
	.num_ranges = ARRAY_SIZE(aldo4_ranges),
};

__maybe_unused static const struct linear_range bldo1_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 100000U, 0x00U, 0x1EU),
};

__maybe_unused static const struct regulator_axp2101_desc bldo1_desc = {
	.enable_reg = AXP2101_REG_ALDO1234_BLDO12_CPUSLDO_DLDO1_CONTROL,
	.enable_mask = 0x10U,
	.enable_val = 0x10U,
	.vsel_reg = AXP2101_REG_BLDO1_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 300000U,
	.ranges = bldo1_ranges,
	.num_ranges = ARRAY_SIZE(bldo1_ranges),
};

__maybe_unused static const struct linear_range bldo2_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 100000U, 0x00U, 0x1EU),
};

__maybe_unused static const struct regulator_axp2101_desc bldo2_desc = {
	.enable_reg = AXP2101_REG_ALDO1234_BLDO12_CPUSLDO_DLDO1_CONTROL,
	.enable_mask = 0x20U,
	.enable_val = 0x20U,
	.vsel_reg = AXP2101_REG_BLDO2_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 300000U,
	.ranges = bldo2_ranges,
	.num_ranges = ARRAY_SIZE(bldo2_ranges),
};

__maybe_unused static const struct linear_range cpusldo_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 50000U, 0x00U, 0x13U),
};

__maybe_unused static const struct regulator_axp2101_desc cpusldo_desc = {
	.enable_reg = AXP2101_REG_ALDO1234_BLDO12_CPUSLDO_DLDO1_CONTROL,
	.enable_mask = 0x40U,
	.enable_val = 0x40U,
	.vsel_reg = AXP2101_REG_CPUSLDO_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 300000U,
	.ranges = cpusldo_ranges,
	.num_ranges = ARRAY_SIZE(cpusldo_ranges),
};

__maybe_unused static const struct linear_range dldo1_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 100000U, 0x00U, 0x1EU),
};

__maybe_unused static const struct regulator_axp2101_desc dldo1_desc = {
	.enable_reg = AXP2101_REG_ALDO1234_BLDO12_CPUSLDO_DLDO1_CONTROL,
	.enable_mask = 0x80U,
	.enable_val = 0x80U,
	.vsel_reg = AXP2101_REG_DLDO1_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 300000U,
	.ranges = dldo1_ranges,
	.num_ranges = ARRAY_SIZE(dldo1_ranges),
};

__maybe_unused static const struct linear_range dldo2_ranges[] = {
	LINEAR_RANGE_INIT(500000U, 100000U, 0x00U, 0x1EU),
};

__maybe_unused static const struct regulator_axp2101_desc dldo2_desc = {
	.enable_reg = AXP2101_REG_DLDO2_CONTROL,
	.enable_mask = 0x01U,
	.enable_val = 0x01U,
	.vsel_reg = AXP2101_REG_DLDO2_VOLTAGE,
	.vsel_mask = 0x1FU,
	.vsel_bitpos = 0U,
	.max_ua = 300000U,
	.ranges = dldo2_ranges,
	.num_ranges = ARRAY_SIZE(dldo2_ranges),
};

static int axp2101_enable(const struct device *dev)
{
	const struct regulator_axp2101_config *config = dev->config;

	LOG_INST_DBG(config->log, "Enabling regulator");
	LOG_INST_DBG(config->log, "[0x%02x]=0x%02x mask=0x%02x", config->desc->enable_reg,
				 config->desc->enable_val, config->desc->enable_mask);

	int ret = i2c_reg_update_byte_dt(&config->i2c, config->desc->enable_reg,
									 config->desc->enable_mask, config->desc->enable_val);

	if (ret != 0)
	{
		LOG_INST_ERR(config->log, "Failed to enable regulator");
	}

	return ret;
}

static int axp2101_disable(const struct device *dev)
{
	const struct regulator_axp2101_config *config = dev->config;

	LOG_INST_DBG(config->log, "Disabling regulator");
	LOG_INST_DBG(config->log, "[0x%02x]=0 mask=0x%x", config->desc->enable_reg,
				 config->desc->enable_mask);

	int ret = i2c_reg_update_byte_dt(&config->i2c, config->desc->enable_reg,
									 config->desc->enable_mask, 0u);

	if (ret != 0)
	{
		LOG_INST_ERR(config->log, "Failed to disable regulator");
	}

	return ret;
}

static unsigned int axp2101_count_voltages(const struct device *dev)
{
	const struct regulator_axp2101_config *config = dev->config;

	return linear_range_group_values_count(config->desc->ranges, config->desc->num_ranges);
}

static int axp2101_list_voltage(const struct device *dev, unsigned int idx, int32_t *volt_uv)
{
	const struct regulator_axp2101_config *config = dev->config;

	return linear_range_group_get_value(config->desc->ranges, config->desc->num_ranges, idx,
										volt_uv);
}

static int axp2101_set_voltage(const struct device *dev, int32_t min_uv, int32_t max_uv)
{
	const struct regulator_axp2101_config *config = dev->config;
	uint16_t idx;
	int ret;

	LOG_INST_DBG(config->log, "voltage = [min=%d, max=%d]", min_uv, max_uv);

	// set voltage
	ret = linear_range_group_get_win_index(config->desc->ranges, config->desc->num_ranges,
										   min_uv, max_uv, &idx);
	if (ret != 0)
	{
		LOG_INST_ERR(config->log, "No voltage range window could be detected");
		return ret;
	}

	idx <<= config->desc->vsel_bitpos;

	LOG_INST_DBG(config->log, "[0x%x]=0x%x mask=0x%x", config->desc->vsel_reg, idx,
				 config->desc->vsel_mask);
	ret = i2c_reg_update_byte_dt(&config->i2c, config->desc->vsel_reg, config->desc->vsel_mask,
								 (uint8_t)idx);
	if (ret != 0)
	{
		LOG_INST_ERR(config->log, "Failed to set regulator voltage");
	}

	return ret;
}

static int axp2101_get_voltage(const struct device *dev, int32_t *volt_uv)
{
	const struct regulator_axp2101_config *config = dev->config;
	int ret;
	uint8_t raw_reg;

	// read voltage
	ret = i2c_reg_read_byte_dt(&config->i2c, config->desc->vsel_reg, &raw_reg);
	if (ret != 0)
	{
		return ret;
	}

	raw_reg = (raw_reg & config->desc->vsel_mask) >> config->desc->vsel_bitpos;

	ret = linear_range_group_get_value(config->desc->ranges, config->desc->num_ranges, raw_reg,
									   volt_uv);

	return ret;
}

static int axp2101_set_mode(const struct device *dev, regulator_mode_t mode)
{
	const struct regulator_axp2101_config *config = dev->config;
	int ret;

	// setting workmode is only possible for DCDC1-4
	if ((mode == AXP2101_DCDC_MODE_PWM) && (config->desc->workmode_reg != 0))
	{

		// configure PWM mode
		LOG_INST_DBG(config->log, "PWM mode enabled");
		ret = i2c_reg_update_byte_dt(&config->i2c, config->desc->workmode_reg,
									 config->desc->workmode_mask,
									 config->desc->workmode_pwm_val);
		if (ret != 0)
		{
			return ret;
		}
	}
	else if (mode == AXP2101_DCDC_MODE_AUTO)
	{

		// configure AUTO mode (default)
		if (config->desc->workmode_reg != 0)
		{
			ret = i2c_reg_update_byte_dt(&config->i2c, config->desc->workmode_reg,
										 config->desc->workmode_mask, 0u);
			if (ret != 0)
			{
				return ret;
			}
		}
		else
		{

			// AUTO is default mode for LDOs that cannot be configured
			return 0;
		}
	}
	else
	{
		LOG_INST_ERR(config->log, "Setting DCDC workmode failed");
		return -ENOTSUP;
	}

	return 0;
}

static int axp2101_get_current_limit(const struct device *dev, int32_t *curr_ua)
{
	const struct regulator_axp2101_config *config = dev->config;

	*curr_ua = config->desc->max_ua;

	return 0;
}

static struct regulator_driver_api api = {
	.enable = axp2101_enable,
	.disable = axp2101_disable,
	.count_voltages = axp2101_count_voltages,
	.list_voltage = axp2101_list_voltage,
	.set_voltage = axp2101_set_voltage,
	.get_voltage = axp2101_get_voltage,
	.set_mode = axp2101_set_mode,
	.get_current_limit = axp2101_get_current_limit,
};

static int regulator_axp2101_init(const struct device *dev)
{
	const struct regulator_axp2101_config *config = dev->config;
	uint8_t enabled_val;
	bool is_enabled;
	int ret = 0;

	regulator_common_data_init(dev);

	if (!device_is_ready(config->mfd))
	{
		LOG_INST_ERR(config->log, "Parent instance not ready!");
		return -ENODEV;
	}

	// read regulator state
	ret = i2c_reg_read_byte_dt(&config->i2c, config->desc->enable_reg, &enabled_val);
	if (ret != 0)
	{
		LOG_INST_ERR(config->log, "Reading enable status failed!");
		return ret;
	}
	is_enabled = ((enabled_val & config->desc->enable_mask) == config->desc->enable_val);
	LOG_INST_DBG(config->log, "is_enabled: %d", is_enabled);

	return regulator_common_init(dev, is_enabled);
}

// parent device must be initialized first
BUILD_ASSERT(CONFIG_REGULATOR_AXP2101_INIT_PRIORITY > CONFIG_AXP2101_INIT_PRIORITY);

#define REGULATOR_AXP2101_DEFINE(node_id, id, name)                                   \
	static struct regulator_axp2101_data data_##id;                                   \
	LOG_INSTANCE_REGISTER(name, node_id, CONFIG_AXP2101_LOG_LEVEL);                   \
	static const struct regulator_axp2101_config config_##id = {                      \
		.common = REGULATOR_DT_COMMON_CONFIG_INIT(node_id),                           \
		.desc = &name##_desc,                                                         \
		.mfd = DEVICE_DT_GET(DT_GPARENT(node_id)),                                    \
		.i2c = I2C_DT_SPEC_GET(DT_GPARENT(node_id)),                                  \
		LOG_INSTANCE_PTR_INIT(log, name, node_id)};                                   \
	DEVICE_DT_DEFINE(node_id, regulator_axp2101_init, NULL, &data_##id, &config_##id, \
					 POST_KERNEL, CONFIG_REGULATOR_AXP2101_INIT_PRIORITY, &api);

#define REGULATOR_AXP2101_DEFINE_COND(inst, child)                                          \
	COND_CODE_1(DT_NODE_EXISTS(DT_INST_CHILD(inst, child)),                                 \
				(REGULATOR_AXP2101_DEFINE(DT_INST_CHILD(inst, child), child##inst, child)), \
				())

#define REGULATOR_AXP2101_DEFINE_ALL(inst)     \
	REGULATOR_AXP2101_DEFINE_COND(inst, dcdc1) \
	REGULATOR_AXP2101_DEFINE_COND(inst, dcdc2) \
	REGULATOR_AXP2101_DEFINE_COND(inst, dcdc3) \
	REGULATOR_AXP2101_DEFINE_COND(inst, dcdc4) \
	REGULATOR_AXP2101_DEFINE_COND(inst, dcdc5) \
	REGULATOR_AXP2101_DEFINE_COND(inst, aldo1) \
	REGULATOR_AXP2101_DEFINE_COND(inst, aldo2) \
	REGULATOR_AXP2101_DEFINE_COND(inst, aldo3) \
	REGULATOR_AXP2101_DEFINE_COND(inst, aldo4) \
	REGULATOR_AXP2101_DEFINE_COND(inst, bldo1) \
	REGULATOR_AXP2101_DEFINE_COND(inst, bldo2) \
	REGULATOR_AXP2101_DEFINE_COND(inst, dldo1) \
	REGULATOR_AXP2101_DEFINE_COND(inst, dldo2) \
	REGULATOR_AXP2101_DEFINE_COND(inst, cpusldo)

DT_INST_FOREACH_STATUS_OKAY(REGULATOR_AXP2101_DEFINE_ALL);
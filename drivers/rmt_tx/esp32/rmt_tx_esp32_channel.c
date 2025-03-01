#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/rmt_tx.h>
#include <zephyr/logging/log.h>

#include <hal/rmt_hal.h>
#include <hal/rmt_ll.h>

LOG_MODULE_DECLARE(rmt_tx_esp32, CONFIG_RMT_TX_LOG_LEVEL);

#define DT_DRV_COMPAT espressif_esp32_rmt_tx

struct rmt_tx_esp32_channel_config
{
    const struct device *parent;
    const struct gpio_dt_spec gpio;
    uint8_t channel;
    bool invert_out;
};

static int rmt_tx_esp32_set_carrier(const struct device *dev, bool carrier_en,
                                    k_timeout_t high_duration, k_timeout_t low_duration,
                                    rmt_tx_carrier_level_t carrier_level)
{
    const struct rmt_tx_esp32_channel_config *config = dev->config;
    rmt_hal_context_t *hal = (rmt_hal_context_t *)config->parent->data;
    rmt_dev_t *rmt_dev = (rmt_dev_t *)hal->regs;
    LOG_DBG("rmt_dev: %p", rmt_dev);

    return -ENOTSUP;
}

static int rmt_tx_esp32_transmit(const struct device *dev, const struct rmt_symbol *symbols,
                                 size_t num_symbols, k_timeout_t timeout)
{
    const struct rmt_tx_esp32_channel_config *config = dev->config;
    rmt_hal_context_t *hal = (rmt_hal_context_t *)config->parent->data;
    rmt_dev_t *rmt_dev = (rmt_dev_t *)hal->regs;
    LOG_DBG("rmt_dev: %p", rmt_dev);

    return -ENOTSUP;
}

static struct rmt_tx_driver_api rmt_tx_esp32_driver_api = {
    .set_carrier = rmt_tx_esp32_set_carrier,
    .transmit = rmt_tx_esp32_transmit,
};

static int rmt_tx_esp32_channel_init(const struct device *dev)
{
    const struct rmt_tx_esp32_channel_config *config = dev->config;

    // parent device handles the initialization of the overall RMT peripheral
    if (!device_is_ready(config->parent))
    {
        LOG_ERR("Parent device not ready");
        return -ENODEV;
    }

    // Configure GPIO
    int ret = gpio_pin_configure_dt(&config->gpio, GPIO_OUTPUT);
    if (ret < 0)
    {
        LOG_ERR("Failed to configure GPIO pin %d", config->gpio.pin);
        return ret;
    }

    // TODO: validate that this is actually all of the initialization
    // necessary per channel. The parent device is in charge of the
    // overall RMT peripheral.

    return 0;
}

// parent node must be initialized first
BUILD_ASSERT(CONFIG_RMT_TX_CHANNEL_INIT_PRIORITY >= CONFIG_RMT_TX_INIT_PRIORITY);

#define RMT_TX_ESP32_CHANNEL_DEFINE(node)                                        \
    BUILD_ASSERT(DT_REG_ADDR(node) < 4);                                         \
    static const struct rmt_tx_esp32_channel_config config##node = {             \
        .parent = DEVICE_DT_GET(DT_PARENT(node)),                                \
        .gpio = GPIO_DT_SPEC_GET(node, gpios),                                   \
        .channel = DT_REG_ADDR(node),                                            \
        .invert_out = DT_PROP(node, invert_out),                                 \
    };                                                                           \
    DEVICE_DT_DEFINE(node, rmt_tx_esp32_channel_init, NULL, NULL, &config##node, \
                     POST_KERNEL, CONFIG_RMT_TX_CHANNEL_INIT_PRIORITY,           \
                     &rmt_tx_esp32_driver_api);

#define RMT_TX_ESP32_CHANNEL_DEFINE_ALL(inst) \
    DT_FOREACH_CHILD(DT_DRV_INST(inst), RMT_TX_ESP32_CHANNEL_DEFINE)

DT_INST_FOREACH_STATUS_OKAY(RMT_TX_ESP32_CHANNEL_DEFINE_ALL)

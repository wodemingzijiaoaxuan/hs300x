/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-03     flybreak     the first version
 */

#include <stdint.h>
#include <stdlib.h>

#include "sensor_renesas_hs300x.h"

#define DBG_TAG "sensor.renesas.hs300x"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>

#if defined(RT_VERSION_CHECK)
    #if (RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 2))
        #define RT_SIZE_TYPE   rt_ssize_t
    #else
        #define RT_SIZE_TYPE   rt_size_t
    #endif
#endif

#define SENSOR_TEMP_RANGE_MAX (80)
#define SENSOR_TEMP_RANGE_MIN (-10)
#define SENSOR_HUMI_RANGE_MAX (100)
#define SENSOR_HUMI_RANGE_MIN (0)

/* Definitions of Mask Data for A/D data */
#define RM_HS300X_MASK_HUMIDITY_UPPER_0X3F       (0x3F)
#define RM_HS300X_MASK_TEMPERATURE_LOWER_0XFC    (0xFC)
#define RM_HS300X_MASK_STATUS_0XC0               (0xC0)

/* Definitions for Status Bits of A/D Data */
#define RM_HS300X_DATA_STATUS_VALID              (0x00) // Status-bit: Valid data

/* Definitions for Calculation */
#define RM_HS300X_CALC_STATIC_VALUE              (16383.0F)
#define RM_HS300X_CALC_HUMD_VALUE_100            (100.0F)
#define RM_HS300X_CALC_TEMP_C_VALUE_165          (165.0F)
#define RM_HS300X_CALC_TEMP_C_VALUE_40           (40.0F)
#define RM_HS300X_CALC_DECIMAL_VALUE_100         (100.0F)

struct hs300x_device
{
    struct rt_i2c_bus_device *i2c;

    uint8_t addr;
};
static struct hs300x_device temp_humi_dev;

static rt_err_t _hs300x_init(struct rt_sensor_intf *intf)
{
    temp_humi_dev.i2c = rt_i2c_bus_device_find(intf->dev_name);
    temp_humi_dev.addr = (uint8_t)intf->user_data;

    if (temp_humi_dev.i2c == RT_NULL)
    {
        return -RT_ERROR;
    }

    return RT_EOK;
}

int hs300x_read_data(struct hs300x_device* dev, float *humi_f, float *temp_f)
{
    uint8_t r_buf[4] = {0};
    uint16_t humi, temp;
    float    tmp_f   = 0.0;
    struct rt_i2c_msg msgs;

    msgs.addr = dev->addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf = 0;
    msgs.len = 0;
    if (rt_i2c_transfer(dev->i2c, &msgs, 1) != 1)
    {
        LOG_E("mr error!\n");
        return -1;
    }

    msgs.addr = dev->addr;
    msgs.flags = RT_I2C_RD;
    msgs.buf = r_buf;
    msgs.len = 4;

__retry:
    if (rt_i2c_transfer(dev->i2c, &msgs, 1) == 1)
    {
        LOG_D("%x,%x,%x,%x\n", r_buf[0], r_buf[1], r_buf[2], r_buf[3]);
        LOG_D("state:%x\n", r_buf[0] & RM_HS300X_MASK_STATUS_0XC0);
        if ((r_buf[0] & RM_HS300X_MASK_STATUS_0XC0) != RM_HS300X_DATA_STATUS_VALID)
        {
            rt_thread_mdelay(100);
            goto __retry;
        }

        humi = (r_buf[0] & RM_HS300X_MASK_HUMIDITY_UPPER_0X3F) << 8 | r_buf[1];
        temp = (r_buf[2] << 8 | (r_buf[3] & RM_HS300X_MASK_TEMPERATURE_LOWER_0XFC)) >> 2;

        tmp_f = (float)humi;
        tmp_f = (tmp_f * RM_HS300X_CALC_HUMD_VALUE_100) / RM_HS300X_CALC_STATIC_VALUE;
        if (humi_f)
        {
            *humi_f = tmp_f;
        }

        tmp_f = (float)temp;
        tmp_f = ((tmp_f * RM_HS300X_CALC_TEMP_C_VALUE_165) / RM_HS300X_CALC_STATIC_VALUE) - RM_HS300X_CALC_TEMP_C_VALUE_40;
        if (temp_f)
        {
            *temp_f = tmp_f;
        }
        return 0;
    }
    else {
        LOG_E("read error!\n");
        return -1;
    }
}

float hs300x_read_temperature(struct hs300x_device* dev)
{
    float temp;
    if (hs300x_read_data(dev, NULL, &temp) == 0)
    {
        return temp;
    }
    return 0;
}

float hs300x_read_humidity(struct hs300x_device* dev)
{
    float humi;
    if (hs300x_read_data(dev, &humi, NULL) == 0)
    {
        return humi;
    }
    return 0;
}

static rt_size_t _hs300x_polling_get_data(rt_sensor_t sensor, struct rt_sensor_data *data)
{
    float temperature_x10, humidity_x10;

    if (sensor->info.type == RT_SENSOR_CLASS_TEMP)
    {
        temperature_x10 = 10 * hs300x_read_temperature(&temp_humi_dev);
        data->data.temp = (rt_int32_t)temperature_x10;
        data->timestamp = rt_sensor_get_ts();
    }
    else if (sensor->info.type == RT_SENSOR_CLASS_HUMI)
    {
        humidity_x10    = 10 * hs300x_read_humidity(&temp_humi_dev);
        data->data.humi = (rt_int32_t)humidity_x10;
        data->timestamp = rt_sensor_get_ts();
    }
    return 1;
}

static RT_SIZE_TYPE hs300x_fetch_data(struct rt_sensor_device *sensor, void *buf, rt_size_t len)
{
    RT_ASSERT(buf);

    if (sensor->config.mode == RT_SENSOR_MODE_POLLING)
    {
        return _hs300x_polling_get_data(sensor, buf);
    }
    else
        return 0;
}

static rt_err_t hs300x_control(struct rt_sensor_device *sensor, int cmd, void *args)
{
    rt_err_t result = RT_EOK;

    return result;
}

static struct rt_sensor_ops sensor_ops =
{
    hs300x_fetch_data,
    hs300x_control
};

int rt_hw_hs300x_init(const char *name, struct rt_sensor_config *cfg)
{
    rt_int8_t result;
    rt_sensor_t sensor_temp = RT_NULL, sensor_humi = RT_NULL;

     /* temperature sensor register */
    sensor_temp = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_temp == RT_NULL)
        return -1;

    sensor_temp->info.type       = RT_SENSOR_CLASS_TEMP;
    sensor_temp->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor_temp->info.model      = "hs300x";
    sensor_temp->info.unit       = RT_SENSOR_UNIT_DCELSIUS;
    sensor_temp->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_temp->info.range_max  = SENSOR_TEMP_RANGE_MAX;
    sensor_temp->info.range_min  = SENSOR_TEMP_RANGE_MIN;
    sensor_temp->info.period_min = 5;

    rt_memcpy(&sensor_temp->config, cfg, sizeof(struct rt_sensor_config));
    sensor_temp->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_temp, name, RT_DEVICE_FLAG_RDONLY, RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    /* humidity sensor register */
    sensor_humi = rt_calloc(1, sizeof(struct rt_sensor_device));
    if (sensor_humi == RT_NULL)
        return -1;

    sensor_humi->info.type       = RT_SENSOR_CLASS_HUMI;
    sensor_humi->info.vendor     = RT_SENSOR_VENDOR_UNKNOWN;
    sensor_humi->info.model      = "hs300x";
    sensor_humi->info.unit       = RT_SENSOR_UNIT_PERMILLAGE;
    sensor_humi->info.intf_type  = RT_SENSOR_INTF_I2C;
    sensor_humi->info.range_max  = SENSOR_HUMI_RANGE_MAX;
    sensor_humi->info.range_min  = SENSOR_HUMI_RANGE_MIN;
    sensor_humi->info.period_min = 5;

    rt_memcpy(&sensor_humi->config, cfg, sizeof(struct rt_sensor_config));
    sensor_humi->ops = &sensor_ops;

    result = rt_hw_sensor_register(sensor_humi, name, RT_DEVICE_FLAG_RDONLY, RT_NULL);
    if (result != RT_EOK)
    {
        LOG_E("device register err code: %d", result);
        goto __exit;
    }

    _hs300x_init(&cfg->intf);
    return RT_EOK;

__exit:
    if (sensor_temp)
        rt_free(sensor_temp);
    if (sensor_humi)
        rt_free(sensor_humi);

    return -RT_ERROR;
}

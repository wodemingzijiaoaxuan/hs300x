/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-03     flybreak     the first version
 */

#ifndef SENSOR_RENESAS_HS300X_H__
#define SENSOR_RENESAS_HS300X_H__

#include "rtdevice.h"

#define HS300X_I2C_ADDR 0x44

int rt_hw_hs300x_init(const char *name, struct rt_sensor_config *cfg);

#endif

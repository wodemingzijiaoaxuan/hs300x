# HS300X

## 简介

HS300X 软件包提供了使用温度与湿度传感器 `hs300x` 基本功能。并且本软件包已经对接到了 Sensor 框架，通过 Sensor 框架，开发者可以快速的将此传感器驱动起来。

基本功能主要由传感器 `hs300x` 决定：在输入电压为 `1.8v-3.3v` 范围内，测量温度与湿度的量程、精度如下表所示

| 功能 | 量程 | 精度 |
| ---- | ---- | ---- |
| 温度 | `-10℃ - 80℃` |`±0.2°C(3.3v)`|
| 湿度 | `0% - 100%` |`0.01%(3.3v)`|

## 支持情况

| 包含设备 | 温度 | 湿度 |
| ---- | ---- | ---- |
| **通信接口** |          |        |
| IIC      | √        | √      |
| **工作模式**     |          |        |
| 轮询             | √        | √      |
| 中断             |          |        |
| FIFO             |          |        |

## 使用说明

### 依赖

- RT-Thread 4.0.0+
- Sensor 组件
- IIC 驱动：hs300x 设备使用 IIC 进行数据通讯，需要系统 IIC 驱动支持

### 获取软件包

使用 hs300x 软件包需要在 RT-Thread 的包管理中选中它，具体路径如下：

```
RT-Thread online packages  --->
  peripheral libraries and drivers  --->
    sensors drivers  --->
            hs300x: digital humidity and temperature sensor hs300x driver library.     
                               Version (latest)  --->
```

**Version**：软件包版本选择，默认选择最新版本。

### 使用软件包

hs300x 软件包初始化函数如下所示：

```
int rt_hw_hs300x_init(const char *name, struct rt_sensor_config *cfg)；
```

该函数需要由用户调用，函数主要完成的功能有，

- 设备配置和初始化（根据传入的配置信息配置接口设备）；
- 注册相应的传感器设备，完成 hs300x 设备的注册；

#### 初始化示例

```c
#include "sensor_renesas_hs300x.h"
#define HS300X_I2C_BUS  "i2c2"

int rt_hw_hs300x_port(void)
{
    struct rt_sensor_config cfg;

    cfg.intf.dev_name  = HS300X_I2C_BUS;
    cfg.intf.user_data = (void *)HS300X_I2C_ADDR;
    
    rt_hw_hs300x_init("hs300x", &cfg);

    return RT_EOK;
}
INIT_ENV_EXPORT(rt_hw_hs300x_port);
```

#### 读取数据

- 通过导出的测试命令 `sensor_polling humi_hs3` ，判断能否成功读取湿度数据。
- 通过导出的测试命令 `sensor_polling temp_hs3` ，判断能否成功读取温度数据。

## 注意事项

- 无

## 联系人信息

维护人:

- 维护：[flyboy](https://github.com/Guozhanxin)
- 主页：https://github.com/Guozhanxin/hs300x

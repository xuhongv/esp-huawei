# 安信可Wi-Fi模组对接华为云物联网平台的指南
# 目录

- [0.介绍](#Introduction)  
- [1.目的](#aim)  
- [2.硬件准备](#hardwareprepare)  
- [3.华为云IoT平台准备](#aliyunprepare)  
- [4.环境搭建](#compileprepare)  
- [5.SDK 准备](#sdkprepare)  
- [6.编译&烧写&运行](#makeflash)  

# <span id = "Introduction">0.介绍</span>
[安信可](https://www.espressif.com/zh-hans)是物联网无线的设计专家，专注于设计简单灵活、易于制造和部署的解决方案。安信可研发和设计 IoT 业内集成度SoC、性能稳定、功耗低的无线系统级模组产品，因此具备强大的 Wi-Fi 和蓝牙功能，以及出色的射频性能。

[华为云物联网平台](https://support.huaweicloud.com/productdesc-iothub/iot_04_0002.html)（IoT 设备接入云服务）提供海量设备的接入和管理能力，可以将您的IoT设备联接到华为云，支撑设备数据采集上云和云端下发命令给设备进行远程控制，配合华为云其他产品，帮助您快速构筑物联网解决方案。

使用物联网平台构建一个完整的物联网解决方案主要包括3部分：物联网平台、业务应用和设备。

- 物联网平台作为连接业务应用和设备的中间层，屏蔽了各种复杂的设备接口，实现设备的快速接入；同时提供强大的开放能力，支撑行业用户快速构建各种物联网业务应用。
- 设备可以通过固网、2G/3G/4G/5G、NB-IoT、Wifi等多种网络接入物联网平台，并使用LWM2M/CoAP或MQTT协议将业务数据上报到平台，平台也可以将控制命令下发给设备。
- 业务应用通过调用物联网平台提供的API，实现设备数据采集、命令下发、设备管理等业务场景。

# <span id = "aim">1.目的</span>
本文基于 linux 环境，介绍安信可Wi-Fi模组对接华为云物联网平台的具体流程，供读者参考。

| 安信可在售模组                              | 是否支持            |
| ------------------------------------------- | ------------------- |
| ESP8266系列模组，包括ESP-12F/12S/12E/12L    | 暂不支持，适配中... |
| ESP32系列模组，包括ESP32-S、ESP32-SU        | 支持                |
| ESP32S2系列模组，包括ESP-12K、ESP-12H       | 支持                |
| ESP32S3系列模组，包括ESP-S3-12K、ESP-S3-12F | 暂不支持，适配中... |
| ESP32C3系列模组，包括ESP-C3-32S、ESP-C3-12F | 支持                |

# <span id = "hardwareprepare">2.硬件准备</span>
- **linux 环境**  
用来编译 & 烧写 & 运行等操作的必须环境。 
> windows 用户可安装虚拟机，在虚拟机中安装 linux。

- **设备**  
前往安信可官方获取：[样品](https://anxinke.taobao.com)

- **USB 线**  
连接 PC 和 ESP 设备，用来烧写/下载程序，查看 log 等。

# <span id = "aliyunprepare">3.华为云平台准备</span>
根据[华为云物联网平台接入官方文档](https://support.huaweicloud.com/productdesc-iothub/iot_04_0002.html)，在华为云平台创建产品，创建设备，同时自动产生 将在 6.2.3 节用到。

# <span id = "compileprepare">4.环境搭建</span>
**如果您熟悉 ESP 开发环境，可以很顺利理解下面步骤; 如果您不熟悉某个部分，比如编译，烧录，需要您结合官方的相关文档来理解。如您需阅读 [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/index.html)文档等。**  

## 4.1 编译器环境搭建
- ESP32/s2/c3  ：根据[官方链接](https://github.com/espressif/esp-idf/blob/master/docs/zh_CN/get-started/linux-setup.rst)中 **工具链的设置**，下载 toolchain

toolchain 设置参考 [ESP-IDF 编程指南](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/get-started/index.html#get-started-setup-toolchain)。  
## 4.2 烧录工具/下载工具获取
- ESP32/s2/c3 平台：烧录工具位于 [esp-idf](https://github.com/espressif/esp-idf) 下 `./components/esptool_py/esptool/esptool.py`

esptool 功能参考:  

```
$ ./components/esptool_py/esptool/esptool.py --help
```

# <span id = "sdkprepare">5.SDK 准备</span> 
- [esp-huawei SDK](https://github.com/xuhongv/esp-huawei), 通过该 SDK 可实现使用 MQTT 协议，连接 ESP 设备到华为云物联网平台。
- Espressif SDK
  - ESP32/s2/c3 平台: [ESP-IDF](https://github.com/espressif/esp-idf)
> Espressif SDK 下载好后：  
> ESP-IDF: 请切换到 v4.3 分支： `git checkout v4.3`

# <span id = "makeflash">6.编译 & 烧写 & 运行</span>
## 6.1 编译

### 6.1.1 导出编译器
参考 [工具链的设置](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/get-started/linux-setup.html)

### 6.1.2 编译 demo 示例
```
idf.py set-target esp32/s2/c3
idf.py menuconfig
```

![p1](static/01.png)

- 配置烧写串口
- 配置从华为物联网获取到的三元组。
- 编译，打开安信可微信公众号进行无线配置入网，即微信airkiss配网协议。

2.生成最终 bin

```
idf.py build
```

## 6.2 擦除 & 编译烧写 & 下载固件 & 查看 log
将 USB 线连接好设备和 PC,确保烧写端口正确。 

### 6.2.1 [可选] 擦除 flash
```
idf.py -p (PORT) erase_flash
```
> 注：无需每次擦除，擦除后需要重做 6.2.3。

### 6.2.2 烧录程序
```
idf.py -p (PORT) _flash
```

### 6.2.3 量产烧录三元组信息
- 待完成说明。。

## 6.2.4 运行

```
make monitor
```

如将 ESP32 拨至运行状态，即可看到如下 log：
log 显示了 ESP32 基于 TLS 建立了与华为云的安全连接通路，接着通过 MQTT 协议订阅和发布消息，同时在控制台上，也能看到 ESP32 推送的 MQTT 消息。  

```
I (798) wifi:enable tsf
I (808) wifi_init: rx ba win: 6
I (808) wifi_init: tcpip mbox: 32
I (808) wifi_init: udp mbox: 6
I (808) wifi_init: tcp mbox: 6
I (808) wifi_init: tcp tx win: 5744
I (808) wifi_init: tcp rx win: 5744
I (818) wifi_init: tcp mss: 1440
I (818) wifi_init: WiFi IRAM OP enabled
I (818) wifi_init: WiFi RX IRAM OP enabled
I (828) router: -- get ssid: aiot@xuhongv
I (828) router: -- get password: xuhong12345678
I (838) aithinker-debugLog::: Next connectting router.
I (838) wifi:new:<1,0>, old:<1,0>, ap:<255,255>, sta:<1,0>, prof:1
I (848) wifi:state: init -> auth (b0)
I (898) wifi:state: auth -> assoc (0)
I (938) wifi:state: assoc -> run (10)
I (958) wifi:connected with aiot@xuhongv, aid = 3, channel 1, BW20, bssid = 9c:9d:7e:40:e8:10
I (958) wifi:security: WPA2-PSK, phy: bgn, rssi: -21
I (968) wifi:pm start, type: 1

W (968) wifi:<ba-add>idx:0 (ifx:0, 9c:9d:7e:40:e8:10), tid:6, ssn:2, winSize:64
I (988) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (2088) esp_netif_handlers: sta ip: 192.168.31.228, mask: 255.255.255.0, gw: 192.168.31.1
I (2088) aithinker-debugLog::: WiFi Connected to ap
I (2088) aithinker-debugLog::: Free memory: 229384 bytes
I (2088) aithinker-debugLog::: MQTT xTaskCreate OK

I (2098) aithinker-debugLog::: port 8883
I (2098) aithinker-debugLog::: host  a1621cfafc.iot-mqtts.cn-north-4.myhuaweicloud.com
I (2108) aithinker-debugLog::: client_id aithinker_F4CFA25BB155_0_0_2022032704
I (2118) aithinker-debugLog::: username aithinker_F4CFA25BB155
I (2118) aithinker-debugLog::: password3e7bbb8fba3161685348d96434bb43fb450f39d765232ffbcd8ecb35df2be35b
I (2128) aithinker-debugLog::: Other event id:7
W (2148) wifi:<ba-add>idx:1 (ifx:0, 9c:9d:7e:40:e8:10), tid:0, ssn:0, winSize:64
I (3458) aithinker-debugLog::: MQTT_EVENT_CONNECTED
I (3458) aithinker-debugLog::: sent subscribe successful=$oc/devices/aithinker_F4CFA25BB195/sys/events/down
I (3468) aithinker-debugLog::: sent subscribe successful=$oc/devices/aithinker_F4CFA25BB195/sys/messages/dowm
I (3558) aithinker-debugLog::: MQTT_EVENT_SUBSCRIBED, msg_id=60971
I (3618) aithinker-debugLog::: MQTT_EVENT_SUBSCRIBED, msg_id=21864
```

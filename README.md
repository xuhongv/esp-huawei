# 安信可Wi-Fi模组对接华为云物联网平台的指南
# 目录

- [0.介绍](#Introduction)  
- [1.目的](#aim)  
- [2.硬件准备](#hardwareprepare)  
- [3.华为云IoT平台准备](#aliyunprepare)  
- [4.环境搭建](#compileprepare)  
- [5.SDK 准备](#sdkprepare)  
- [6.编译&烧写&运行](#makeflash)  
- [7.OTA演示](#ota)  

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

- 默认上电的配网日志打印

```
I (885) wifi:mode : sta (9c:9c:1f:1d:33:00)
I (895) wifi:enable tsf
I (895) wifi_init: rx ba win: 6
I (895) wifi_init: tcpip mbox: 32
I (895) wifi_init: udp mbox: 6
I (895) wifi_init: tcp mbox: 6
I (895) wifi_init: tcp tx win: 5744
I (905) wifi_init: tcp rx win: 5744
I (905) wifi_init: tcp mss: 1440
I (905) wifi_init: WiFi IRAM OP enabled
I (905) wifi_init: WiFi RX IRAM OP enabled
I (915) aithinker-debugLog::: into smartconfig mode
I (965) smartconfig: SC version: V3.0.1
I (5065) wifi:ic_enable_sniffer
I (5065) smartconfig: Start to find channel...
I (5065) aithinker-debugLog::: Scan done
I (38435) smartconfig: TYPE: AIRKISS
I (38435) smartconfig: T|AP MAC: 9c:9d:7e:40:e8:10
I (38435) smartconfig: Found channel on 11-0. Start to get ssid and password...
I (38435) aithinker-debugLog::: Found channel
I (41025) smartconfig: T|pswd: xuhongLove
I (41025) smartconfig: T|ssid: aiot@xuhongv
I (41025) wifi:ic_disable_sniffer
I (41025) aithinker-debugLog::: Got SSID and password
I (41035) aithinker-debugLog::: SSID:aiot@xuhongv
I (41035) aithinker-debugLog::: PASSWORD:xuhongLove
I (42295) wifi:new:<11,0>, old:<11,0>, ap:<255,255>, sta:<11,0>, prof:1
I (43435) wifi:state: init -> auth (b0)
I (43495) wifi:state: auth -> assoc (0)
I (43535) wifi:state: assoc -> run (10)
I (43585) wifi:connected with aiot@xuhongv, aid = 2, channel 11, BW20, bssid = 9c:9d:7e:40:e8:10
I (43585) wifi:security: WPA2-PSK, phy: bgn, rssi: -32
I (43595) wifi:pm start, type: 1
```
- 下面 log 显示了 ESP32 基于 TLS 建立了与华为云的安全连接通路，接着通过 MQTT 协议订阅和发布消息，同时在控制台上，也能看到 ESP32 推送的 MQTT 消息。**可在控制台---监控运维---消息跟踪，可看到设备消息上报。  **

```
...
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

... ...

 cloud device_info.topic_common_sub: $oc/devices/aithinker_F4CFA25BB155/sys/commands/request_id=
I (3535) aithinker-debugLog::: sent subscribe successful=$oc/devices/aithinker_F4CFA25BB155/sys/events/down
I (3545) aithinker-debugLog::: sent subscribe successful=$oc/devices/aithinker_F4CFA25BB155/sys/messages/dowm
I (3545) aithinker-debugLog:::  ota public topic: $oc/devices/aithinker_F4CFA25BB155/sys/events/up
I (3555) aithinker-debugLog:::  ota public msg: {"services":[{"service_id":"$ota","event_type":"version_report","paras":{"sw_version":"V1.0","fw_version":"V1.0"}}]}
I (3585) aithinker-debugLog::: MQTT_EVENT_SUBSCRIBED, msg_id=34835
I (3655) aithinker-debugLog::: MQTT_EVENT_SUBSCRIBED, msg_id=22635
I (3655) aithinker-debugLog::: MQTT_EVENT_PUBLISHED, msg_id=61986
I (6195) aithinker-debugLog::: Free memory: 213688 bytes
I (6195) aithinker-debugLog::: cloud topic[95]:
 $oc/devices/aithinker_F4CFA25BB155/sys/commands/request_id=2a843a13-8801-4545-b408-e40491b54b2c
I (6195) aithinker-debugLog::: cloud payload[81]:
 {"paras":{"SleepEN":"SleepUnable"},"service_id":"Sleep","command_name":"SleepIn"}

I (6215) aithinker-debugLog::: get service_id: Sleep
I (6215) aithinker-debugLog::: get command_name: SleepIn
I (6225) aithinker-debugLog::: get paras:
{
        "SleepEN":      "SleepUnable"
}
I (6225) aithinker-debugLog::: cloud requestId: 2a843a13-8801-4545-b408-e40491b54b2c
I (6235) aithinker-debugLog::: [Upload topic]: $oc/devices/aithinker_F4CFA25BB155/sys/commands/response/request_id=2a843a13-8801-4545-b408-e40491b54b2cI (6245) aithinker-debugLog::: [Upload data]: {"result_code":0}
I (6255) aithinker-debugLog::: Free memory: 213120 bytes
I (6325) aithinker-debugLog::: MQTT_EVENT_PUBLISHED, msg_id=9543

```

# <span id = "ota">7.OTA 演示</span> 

## 7.1 修改固件版本

```
idf.py menuconfig
```

 然后修改版本号，在 Component config --- HUAWEI  IOT MQTT Configuration  --- OTA  Configuration  修改第一项。

![](static/Version-Setting.png)

重新编译成功之后，在工程的 build 文件下面生成一个 850KB左右的 **aithinker-esp32-huawei-iot.bin 文件**；

```
idf.py build
```

## 7.2  把新固件传到服务器

打开控制台，找到 设备--软固件升级--资源部管理--固件列表，点击上传固件。

![](static/ota-setting.png)

## 7.3  发送新固件升级

打开控制台，找到 设备--软固件升级--升级任务，点击新建任务。

![](static/task-ota.png)



观察设备打印日志，可见正在升级：

```
 (44410) aithinker-debugLog::: cloud topic[50]:  
 $oc/devices/aithinker_F4CFA25BB155/sys/events/down
I (44410) aithinker-debugLog::: cloud payload[534]: 
 {"object_device_id":"aithinker_F4CFA25BB155","services":[{"event_type":"firmware_upgrade","service_id":"$ota","event_time":"20220404T080450Z","paras":{"version":"v1.5","url":"https://121.36.42.100:8943/iodm/dev/v2.0/upgradefile/applications/2917ef9003854f769d69026380e92ae7/devices/aithinker_F4CFA25BB155/packages/e2bf30511f0b9f65f17e4c54","file_size":884768,"access_token":"1e766a203ee2b15a567833eb0697a900ff502780eb84c69df9fdbc4d0294145a","expires":86400,"sign":"52683c81688b7d5501713d6f0f0d99ec9ea9a5514aaa4594db079bcfb6d86b26"}}]} 

I (44460) aithinker-debugLog::: pJSON_Item_event_type: firmware_upgrade
E (44470) ota:: get-> bufferOTAToken: Bearer 1e766a203ee2b15a567833eb0697a900ff502780eb84c69df9fdbc4d0294145a
E (44480) ota:: get-> bufferOTAURL: https://121.36.42.100:8943/iodm/dev/v2.0/upgradefile/applications/2917ef9003854f769d69026380e92ae7/devices/aithinker_F4CFA25BB155/packages/e2bf30511f0b9f65f17e4c54
E (44500) ota:: get->bufferOTAVersion: v1.5
I (44500) ota:: get access_token:Bearer 1e766a203ee2b15a567833eb0697a900ff502780eb84c69df9fdbc4d0294145a
I (44510) ota:: get url: https://121.36.42.100:8943/iodm/dev/v2.0/upgradefile/applications/2917ef9003854f769d69026380e92ae7/devices/aithinker_F4CFA25BB155/packages/e2bf30511f0b9f65f17e4c54
I (44530) ota:: get version: v1.5
hostname:121.36.42.100
port:8943
path:/iodm/dev/v2.0/upgradefile/applications/2917ef9003854f769d69026380e92ae7/devices/aithinker_F4CFA25BB155/packages/e2bf30511f0b9f65f17e4c54
I (44540) ota:: pVersion: v1.5
I (44550) ota:: http header: GET  /iodm/dev/v2.0/upgradefile/applications/2917ef9003854f769d69026380e92ae7/devices/aithinker_F4CFA25BB155/packages/e2bf30511f0b9f65f17e4c54  HTTP/1.0

Host:  121.36.42.100 

User-Agent: aithinker/1.0 xuhongv

Content-Type: application/json

Authorization: Bearer 1e766a203ee2b15a567833eb0697a900ff502780eb84c69df9fdbc4d0294145a


I (44580) ota:: Starting OTA example
I (44580) ota:: Running partition type 0 subtype 16 (offset 0x00010000)
I (44590) ota:: Loading the CA root certificate...
I (44790) ota:: Setting hostname for TLS session...
I (44790) ota:: Setting up the SSL/TLS structure...
I (44790) ota:: Connecting to 121.36.42.100:8943...
I (44800) aithinker-debugLog::: huawei_https_ota...
I (44860) ota:: Connected.
I (44860) ota:: Performing the SSL/TLS handshake...
I (45780) ota:: Verifying peer X.509 certificate...
I (45780) ota:: Certificate verified.
I (45780) ota:: Cipher suite is TLS-ECDHE-RSA-WITH-AES-256-GCM-SHA384
I (45790) ota:: Writing HTTP request...
I (45790) ota:: 334 bytes written
I (45790) ota:: Writing to partition subtype 17 at offset 0x110000
I (50010) ota:: esp_ota_begin succeeded
I (50010) ota:: Totoal OTA number 0 update to 1 part
I (50010) ota:: Reading HTTP response...
I (50020) ota:: parse Content-Length:884768, ota_size 884768
I (50030) ota:: Have written image length 721
I (50030) ota:: Have written image length 1745
I (50030) ota:: Have written image length 2493
I (50160) ota:: Have written image length 3517
I (50160) ota:: Have written image length 4541
I (50160) ota:: Have written image length 5565
I (50170) ota:: Have written image length 6589
I (50170) ota:: Have written image length 7613
I (50170) ota:: Have written image length 8637
I (50180) ota:: Have written image length 9661
I (50180) ota:: Have written image length 10685
I (50190) ota:: Have written image length 11709
I (50200) ota:: Have written image length 12733
I (50210) ota:: Have written image length 13709
I (50210) ota:: Have written image length 14733
I (50220) ota:: Have written image length 15111

... ... ... ...

I (61650) ota:: Have written image length 877557
I (61720) ota:: Have written image length 878581
I (61730) ota:: Have written image length 879605
I (61730) ota:: Have written image length 880629
I (61730) ota:: Have written image length 881653
I (61740) ota:: Have written image length 882677
I (61740) ota:: Have written image length 883701
I (61750) ota:: Have written image length 884725
I (61750) ota:: Have written image length 884768
I (61750) ota:: Total Write binary data length : 884768
I (61750) esp_image: segment 0: paddr=00110020 vaddr=3f400020 size=2125ch (135772) map
I (61800) esp_image: segment 1: paddr=00131284 vaddr=3ffb0000 size=041f8h ( 16888) 
I (61810) esp_image: segment 2: paddr=00135484 vaddr=40080000 size=0ab94h ( 43924) 
I (61830) esp_image: segment 3: paddr=00140020 vaddr=400d0020 size=9da38h (645688) map
I (62040) esp_image: segment 4: paddr=001dda60 vaddr=4008ab94 size=0a57ch ( 42364) 
I (62050) esp_image: segment 5: paddr=001e7fe4 vaddr=50000000 size=00010h (    16) 
I (62050) esp_image: segment 0: paddr=00110020 vaddr=3f400020 size=2125ch (135772) map
I (62100) esp_image: segment 1: paddr=00131284 vaddr=3ffb0000 size=041f8h ( 16888) 
I (62110) esp_image: segment 2: paddr=00135484 vaddr=40080000 size=0ab94h ( 43924) 
I (62120) esp_image: segment 3: paddr=00140020 vaddr=400d0020 size=9da38h (645688) map
I (62330) esp_image: segment 4: paddr=001dda60 vaddr=4008ab94 size=0a57ch ( 42364) 
I (62350) esp_image: segment 5: paddr=001e7fe4 vaddr=50000000 size=00010h (    16) 
I (62400) ota:: Prepare to restart system!
I (62400) ota:: Completed 1 requests and  restart 
I (62400) ota:: 3...
I (62460) aithinker-debugLog::: MQTT_EVENT_PUBLISHED, msg_id=40947
I (63400) ota:: 2...
I (64400) ota:: 1...
I (65400) ota:: 0...
I (66400) wifi:state: run -> init (0)
```


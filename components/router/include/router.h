/*
 * @Author: your name
 * @Date: 2022-03-26 09:37:54
 * @LastEditTime: 2022-03-26 10:26:26
 * @LastEditors: your name
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \esp-idf-v4.3\examples\aithinker-esp32-huawei-iot\components\router\include\router.h
 */
#ifndef _IOT_ROUTER_H_
#define _IOT_ROUTER_H_

#include <string.h>
#include <stdlib.h>
#include "nvs_flash.h"
#include "nvs.h"

/**
 * @description:  自动连接路由器
 * @param {type}  是否成功连接！ false：获取ssid失败！true：成功自动连接路由器
 * @return: 
 */
bool routerStartConnect();

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
void router_wifi_clean_info(void);

/**
 * @description: 
 * @param {type} 
 * @return: 
 */
void router_wifi_save_info(uint8_t *ssid, uint8_t *password);

#endif
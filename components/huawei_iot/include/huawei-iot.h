/*
 * @Author: your name
 * @Date: 2022-03-26 09:37:54
 * @LastEditTime: 2022-03-27 13:54:01
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \esp-idf-v4.3\examples\aithinker-esp32-huawei-iot\components\router\include\router.h
 */
#ifndef _IOT_HUWEI_IOT_H_
#define _IOT_HUWEI_IOT_H_

#include <string.h>
#include <stdlib.h>
#include "nvs_flash.h"
#include "nvs.h"
#include "mqtt_client.h"

#define SHA256_HASH_SIZE 32
#define TOPIC_SIZE 80

typedef struct
{
    char password[SHA256_HASH_SIZE * 2 + 1];
    char client_id[50];

    char topic_sub_ota[TOPIC_SIZE];
    char topic_pub_ota[TOPIC_SIZE];

    char topic_sub_messages[TOPIC_SIZE];
    char topic_pub_messages[TOPIC_SIZE];

} hauwei_iot_device_t;
hauwei_iot_device_t device_info;

/**
 *
 *
 *
 *
 *
 **/
void HuaWei_IOT_Cloud_mqtt_params_get();

#endif
/*
 * @Author: your name
 * @Date: 2022-03-26 09:37:54
 * @LastEditTime: 2022-04-04 15:31:24
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

    char topic_common_sub[TOPIC_SIZE];
    char topic_common_pub[TOPIC_SIZE];

    int topic_common_sub_length;
    int topic_common_pub_length;

} hauwei_iot_device_t;
hauwei_iot_device_t device_info;

/**
 * @brief HTTP configuration
 */
typedef struct
{
    char *url;   /*!< url */
    int port;    /*!< Port to connect, default depend on esp_http_client_transport_t (80 or 443) */
    char *path;  /*!< HTTP Path, if not set, default is `/` */
    char *token; /*!< HTTP oauth token `/` */
    char *version;

    int url_length;
    int token_length;
    int version_length;

} huawei_http_client_config_t;

typedef enum
{
    OTA_CODE_ANY = -1,
    OTA_CODE_PROCESS = 0,          //处理成功
    OTA_CODE_USE = 1,              //设备使用中
    OTA_CODE_QOS_NOT_GOOD = 2,     //信号质量差
    OTA_CODE_SUCCESS = 3,          //处理成功
    OTA_CODE_CHANGER_LACK = 4,     //电量不足
    OTA_CODE_FREE_MEMORY_LACK = 5, //剩余空间不足
    OTA_CODE_PROCESS_TIMEOUT = 6,  //下载超时
    OTA_CODE_FILE_CHECK_FAIL = 7,  //升级包校验失败
    OTA_CODE_FILE_NOT_SUPPORT = 8, //升级包类型不支持
    OTA_CODE_MEMORY_LACK = 9,      //内存不足
    OTA_CODE_INSTALL_FAIL = 10,    //安装升级包失败

    OTA_CODE_MODULE_REBOOT = 255, //内部异常

} huawei_mqtt_ota_code_t;

/*
 * @Description:
 * @param:
 * @return:
 */
void HuaWei_IOT_Cloud_mqtt_params_get();

/*
 * @Description: 生成版本信息
 * @param:
 * @return:
 */
void json_generate_version_info(char *pbDest);

/*
 * @Description: 生成ota升级信息
 * @param:
 * @return:
 */
void json_generate_ota_progress_info(char *pbDest, huawei_mqtt_ota_code_t errcode, int progress);

/*
 * @Description: ota成功信息或版本上报
 * @param:
 * @return:
 */
void json_generate_ota_progress_success(char *pbDest, char *pVersion);

/*
 * @Description:
 * @param:
 * @return:
 */
esp_err_t huawei_https_ota(    huawei_http_client_config_t *config);

#endif
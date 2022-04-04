/*
 * @Author: your name
 * @Date: 2022-03-26 10:43:44
 * @LastEditTime: 2022-04-04 14:26:30
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \esp-idf-v4.3\examples\aithinker-esp32-huawei-iot\components\huawei_iot\huawei-iot.c
 */

#include "huawei-iot.h"
#include "esp_log.h"
#include "mbedtls/md.h"

static void hmac_sha256_calc_result(unsigned char *pDigest, unsigned char *secret, int secret_length, unsigned char *buffer, int buffer_length)
{
    int ret;
    mbedtls_md_context_t sha_ctx;

    mbedtls_md_init(&sha_ctx);

    ret = mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1);
    if (ret != 0)
    {
        printf("  ! mbedtls_md_setup() returned -0x%04x\n", -ret);
        goto exit;
    }
    mbedtls_md_hmac_starts(&sha_ctx, secret, secret_length);
    mbedtls_md_hmac_update(&sha_ctx, buffer, buffer_length);
    mbedtls_md_hmac_finish(&sha_ctx, pDigest);

exit:
    mbedtls_md_free(&sha_ctx);
}

// 华为云计算工具
// https://iot-tool.obs-website.cn-north-4.myhuaweicloud.com/
// https://iodps-file.obs.cn-north-4.myhuaweicloud.com/tools/iotprovisioning.html

// CONFIG_HUAWEI_HARDWARE_VERSION="V1.0"
// CONFIG_HUAWEI_SOFTWARE_VERSION="V1.0"
// # end of OTA Configuration

// CONFIG_HUWEI_IOT_ENABLE_MQTTS=y

// #
// # HUWEI IOT PV Configuration
// #
// # CONFIG_HUWEI_IOT_PV_ENABLE is not set

// #
// # HUWEI IOT Triples Configuration
// #
// CONFIG_MQTT_HOST="xxxxxx.iot-mqtts.cn-north-4.myhuaweicloud.com"
// CONFIG_TRIPLES_DEVICE_SN_CODE="F4CFA25BB155"
// CONFIG_TRIPLES_DEVICE_ID="aithinker_F4CFA25BB155"
// CONFIG_TRIPLES_DEVICE_SECRET="F4CFA25BB155"

void HuaWei_IOT_Cloud_mqtt_params_get()
{

    unsigned char *pDigest = (unsigned char *)malloc(100);

// 量产模式
#ifdef CONFIG_HUWEI_IOT_PV_ENABLE
    //从指定的flash读取数据
    unsigned char secret[] = "2022032704";
    unsigned char buffer[] = CONFIG_TRIPLES_DEVICE_SECRET;
#else
    //从配置文件里面读取数据
    unsigned char secret[] = "2022032704";
    unsigned char buffer[] = CONFIG_TRIPLES_DEVICE_SECRET;
#endif

    hmac_sha256_calc_result(pDigest, secret, strlen((char *)secret), buffer, strlen((char *)buffer));
    for (uint8_t i = 0; i < strlen((const char *)pDigest); i++)
    {
        snprintf(&device_info.password[i * 2], 3, "%02x", pDigest[i]);
    }

    device_info.password[SHA256_HASH_SIZE * 2] = '\0';
    sprintf(device_info.client_id, "%s_0_0_2022032704", CONFIG_TRIPLES_DEVICE_ID);

    sprintf(device_info.topic_sub_messages, "$oc/devices/%s/sys/messages/dowm", CONFIG_TRIPLES_DEVICE_ID);
    sprintf(device_info.topic_pub_messages, "$oc/devices/%s/sys/messages/up", CONFIG_TRIPLES_DEVICE_ID);

    sprintf(device_info.topic_pub_ota, "$oc/devices/%s/sys/events/up", CONFIG_TRIPLES_DEVICE_ID);
    sprintf(device_info.topic_sub_ota, "$oc/devices/%s/sys/events/down", CONFIG_TRIPLES_DEVICE_ID);

    sprintf(device_info.topic_common_sub, "$oc/devices/%s/sys/commands/request_id=", CONFIG_TRIPLES_DEVICE_ID);
    sprintf(device_info.topic_common_pub, "$oc/devices/%s/sys/commands/response/request_id=", CONFIG_TRIPLES_DEVICE_ID);

    device_info.topic_common_pub_length = strlen(device_info.topic_common_pub);
    device_info.topic_common_sub_length = strlen(device_info.topic_common_sub);

    free(pDigest);
}

void json_generate_version_info(char *pbDest)
{
    sprintf(pbDest, "{\"services\":[{\"service_id\":\"$ota\",\"event_type\":\"version_report\",\"paras\":{\"sw_version\":\"%s\",\"fw_version\":\"%s\"}}]}", CONFIG_HUAWEI_SOFTWARE_VERSION, CONFIG_HUAWEI_HARDWARE_VERSION);
}

void json_generate_ota_progress_info(char *pbDest, huawei_mqtt_ota_code_t errcode, int progress)
{
    sprintf(pbDest, "{\"services\":[{\"service_id\":\"$ota\",\"event_type\":\"upgrade_progress_report\",\"paras\":{\"result_code\":%d,\"progress\":%d,\"version\":\"%s\"}}]}", errcode, progress, CONFIG_HUAWEI_HARDWARE_VERSION);
}

void json_generate_ota_progress_success(char *pbDest, char *pVersion)
{
    sprintf(pbDest, "{\"services\":[{\"service_id\":\"$ota\",\"event_type\":\"upgrade_progress_report\",\"paras\":{\"result_code\":0,\"progress\":100,\"version\":\"%s\"}}]}", pVersion);
}

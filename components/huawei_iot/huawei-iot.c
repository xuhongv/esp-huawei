
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
    char out[SHA256_HASH_SIZE * 2 + 1];
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

    free(pDigest);
}

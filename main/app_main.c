/* File Info
 * Author:      your name
 * CreateTime:  2022/3/25下午11:45:46
 * LastEditor:  your name
 * ModifyTime:  2022/3/26下午8:56:56
 * Description:
 */
#include <stdio.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "cJSON.h"
#include "esp_smartconfig.h"
#include "mbedtls/md.h"

#include "huawei-iot.h"
#include "router.h"

#define TAG "aithinker-debugLog::"
#define HUAWEI_IOT_INFO(fmt, ...) ESP_LOGI(TAG, fmt, ##__VA_ARGS__)
#define HUAWEI_IOT_ERROR(fmt, ...) ESP_LOGE(TAG, fmt, ##__VA_ARGS__)
#define BUFFER_LEN 256

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int NET_DONE_BIT = BIT1;

static TaskHandle_t xMQTTTaskHandle = NULL;
esp_mqtt_client_handle_t client = NULL;

extern const uint8_t mqtt_org_pem_start[] asm("_binary_mqtts_root_cert_pem_start");
extern const uint8_t mqtt_org_pem_end[] asm("_binary_mqtts_root_cert_pem_end");

/*
 * @Description: 上报数据到服务器
 * @param:  buf 内容
 * @param:  len 长度
 * @return:
 */
void publish_data_to_mqtt(char *buf, int32_t len, char *requestId)
{
    // hex2string((char *)buf, bufferOut, 3, src_len);
    char *bufferOut = (char *)malloc(BUFFER_LEN);
    char *pubTopic = (char *)malloc(BUFFER_LEN);
    char strPayload[] = {"{\"result_code\":0}"};

    sprintf(pubTopic, "%s%s", device_info.topic_common_pub, requestId);

    ESP_LOGI(TAG, "[Upload topic]: %s", pubTopic);
    ESP_LOGI(TAG, "[Upload data]: %s", strPayload);
    ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());

    esp_mqtt_client_publish(client, pubTopic, strPayload, strlen(strPayload), 1, 0);

    // Topic：$oc/devices/{device_id}/sys/commands/response/request_id={request_id}
    // 数据格式：
    // {
    //     "result_code": 0,
    //     "response_name": "COMMAND_RESPONSE",
    //     "paras": {
    //         "result": "success"
    //     }
    // }

    free(bufferOut);
    free(pubTopic);
}

/*
 * @Description: MQTT下发的回调函数
 * @param:
 * @return:
 */
static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    client = event->client;
    switch (event->event_id)
    {
    case MQTT_EVENT_CONNECTED:
        HUAWEI_IOT_INFO("MQTT_EVENT_CONNECTED");

        HUAWEI_IOT_INFO("cloud device_info.topic_common_sub: %s", device_info.topic_common_sub);

        esp_mqtt_client_subscribe(client, device_info.topic_sub_ota, 1);
        HUAWEI_IOT_INFO("sent subscribe successful=%s", device_info.topic_sub_ota);
        esp_mqtt_client_subscribe(client, device_info.topic_sub_messages, 1);
        HUAWEI_IOT_INFO("sent subscribe successful=%s", device_info.topic_sub_messages);

        char *bufferOTA = (char *)malloc(BUFFER_LEN);
        json_generate_version_info(bufferOTA);
        HUAWEI_IOT_INFO(" ota public topic: %s", device_info.topic_pub_ota);
        HUAWEI_IOT_INFO(" ota public msg: %s", bufferOTA);
        // 上报版本信息
        esp_mqtt_client_publish(client, device_info.topic_pub_ota, bufferOTA, strlen(bufferOTA), 1, 0);
        free(bufferOTA);

        break;

    case MQTT_EVENT_DISCONNECTED:
        HUAWEI_IOT_INFO("MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:

        HUAWEI_IOT_INFO("MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);

        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        HUAWEI_IOT_INFO("MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        HUAWEI_IOT_INFO("MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
    {
        char *requestId = (char *)malloc(BUFFER_LEN * 2);
        char *payload = (char *)malloc(1024);
        HUAWEI_IOT_INFO("Free memory: %d bytes", esp_get_free_heap_size());

        snprintf((char *)payload, event->data_len + 1, event->data);

        event->topic[event->topic_len] = '\0';
        event->data[event->data_len] = '\0';

        HUAWEI_IOT_INFO("cloud topic[%d]:  \n %s", event->topic_len, event->topic);
        HUAWEI_IOT_INFO("cloud payload[%d]: \n %s \n", event->data_len, payload);

        // 首先整体判断是否为一个json格式的数据
        cJSON *pJsonRoot = cJSON_Parse(payload);
        //如果是否json格式数据
        if (pJsonRoot == NULL)
        {
            printf("[SY] Task_ParseJSON_Message xQueueReceive not json ... \n");
            goto __cJSON_Delete;
        }

        //平台命令下发 https://support.huaweicloud.com/api-iothub/iot_06_v5_3014.html
        if (strstr(event->topic, device_info.topic_common_sub))
        {

            int length = event->topic_len - device_info.topic_common_sub_length;
            strncpy(requestId, event->topic + device_info.topic_common_sub_length, length);
            requestId[length] = '\0';

            cJSON *pJSON_Item_content = cJSON_GetObjectItem(pJsonRoot, "service_id");
            if (cJSON_IsString(pJSON_Item_content))
            {
                HUAWEI_IOT_INFO("get service_id: %s", pJSON_Item_content->valuestring);
            }

            cJSON *pJSON_Item_command_name = cJSON_GetObjectItem(pJsonRoot, "command_name");
            if (cJSON_IsString(pJSON_Item_command_name))
            {
                HUAWEI_IOT_INFO("get command_name: %s", pJSON_Item_command_name->valuestring);
            }

            cJSON *pJSON_Item_paras = cJSON_GetObjectItem(pJsonRoot, "paras");
            if (cJSON_IsObject(pJSON_Item_paras))
            {
                char *p_paras = cJSON_Print(pJSON_Item_paras);
                HUAWEI_IOT_INFO("get paras: \n%s", p_paras);
                cJSON_free(p_paras);
            }

            HUAWEI_IOT_INFO("cloud requestId: %s", requestId);

            // handle your logic
            publish_data_to_mqtt("i am aithinker", 15, requestId);
        }
        //固件升级相关逻辑
        else if (strcmp(event->topic, device_info.topic_sub_ota) == 0)
        {
            cJSON *pJSON_Item_service = cJSON_GetObjectItem(pJsonRoot, "services");
            cJSON *pJSON_Item_service_des = cJSON_GetArrayItem(pJSON_Item_service, 0);
            cJSON *pJSON_Item_event_type = cJSON_GetObjectItem(pJSON_Item_service_des, "event_type");

            ESP_LOGI(TAG, "pJSON_Item_event_type: %s", pJSON_Item_event_type->valuestring);

            if (0 == strcmp("version_query", pJSON_Item_event_type->valuestring))
            {
                char *bufferOTAReport = (char *)malloc(512);
                json_generate_version_info(bufferOTAReport);
                // 上报版本信息
                esp_mqtt_client_publish(client, device_info.topic_pub_ota, bufferOTAReport, strlen(bufferOTAReport), 1, 0);
                free(bufferOTAReport);
            }
            else if (0 == strcmp("firmware_upgrade", pJSON_Item_event_type->valuestring))
            {

                //{
                //     "object_device_id": "aithinker_F4CFA25BB155",
                //     "services": [
                //         {
                //             "event_type": "firmware_upgrade",
                //             "service_id": "$ota",
                //             "event_time": "20220404T070550Z",
                //             "paras": {
                //                 "version": "v1.5",
                //                 "url": "https://121.36.42.100:8943/iodm/dev/v2.0/upgradefile/applications/2917ef9003854f769d69026380e92ae7/devices/aithinker_F4CFA25BB155/packages/e2bf30511f0b9f65f17e4c54",
                //                 "file_size": 884768,
                //                 "access_token": "432308265df07a9df1d1832605c840df3c4015b385750c21353947fba27bf040",
                //                 "expires": 86400,
                //                 "sign": "52683c81688b7d5501713d6f0f0d99ec9ea9a5514aaa4594db079bcfb6d86b26"
                //             }
                //         }
                //     ]
                // }
                cJSON *pJSON_Item_event_paras = cJSON_GetObjectItem(pJSON_Item_service_des, "paras");
                cJSON *pJSON_Item_event_access_token = cJSON_GetObjectItem(pJSON_Item_event_paras, "access_token");
                cJSON *pJSON_Item_event_access_version = cJSON_GetObjectItem(pJSON_Item_event_paras, "version");
                cJSON *pJSON_Item_event_url = cJSON_GetObjectItem(pJSON_Item_event_paras, "url");

                char *bufferOTAToken = (char *)malloc(512);
                sprintf(bufferOTAToken, "Bearer %s", pJSON_Item_event_access_token->valuestring);

                char *bufferOTAURL = (char *)malloc(512);
                memset(bufferOTAURL, 0, 64);
                strncpy(bufferOTAURL, pJSON_Item_event_url->valuestring, strlen(pJSON_Item_event_url->valuestring));

                char *bufferOTAVersion = (char *)malloc(64);
                memset(bufferOTAVersion, 0, 64);
                strncpy(bufferOTAVersion, pJSON_Item_event_access_version->valuestring, strlen(pJSON_Item_event_access_version->valuestring));

                huawei_http_client_config_t config = {
                    .token = bufferOTAToken,
                    .url = bufferOTAURL,
                    .version = bufferOTAVersion,
                };

                config.token[strlen(pJSON_Item_event_access_token->valuestring) + 7] = '\0';
                config.url[strlen(pJSON_Item_event_url->valuestring)] = '\0';
                config.version[strlen(pJSON_Item_event_access_version->valuestring)] = '\0';

                config.url_length = strlen(pJSON_Item_event_url->valuestring);
                config.token_length = strlen(pJSON_Item_event_access_token->valuestring) + 7;
                config.version_length = strlen(pJSON_Item_event_access_version->valuestring);

                if ((huawei_https_ota(&config)) == ESP_OK)
                {
                    ESP_LOGI(TAG, "huawei_https_ota...");
                }
                else
                {
                    char *bufferOTAReportting = (char *)malloc(512);
                    json_generate_ota_progress_info(bufferOTAReportting, OTA_CODE_PROCESS, 10);
                    // 上报进度
                    esp_mqtt_client_publish(client, device_info.topic_pub_ota, bufferOTAReportting, strlen(bufferOTAReportting), 1, 0);
                    free(bufferOTAReportting);
                    ESP_LOGE(TAG, "Firmware Upgrading...");
                }

                free(bufferOTAToken);
                free(bufferOTAURL);
                free(bufferOTAVersion);
            }
        }

    __cJSON_Delete:
        cJSON_Delete(pJsonRoot);

        free(requestId);
        free(payload);
        break;
    }
    case MQTT_EVENT_ERROR:
        HUAWEI_IOT_INFO("MQTT_EVENT_ERROR");
        break;
    default:
        HUAWEI_IOT_INFO("Other event id:%d", event->event_id);
        break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    mqtt_event_handler_cb(event_data);
}

static void TaskMQTTConnect(void *pvParameters)
{

    HuaWei_IOT_Cloud_mqtt_params_get();
    const esp_mqtt_client_config_t mqtt_cfg = {
        .host = CONFIG_MQTT_HOST,
        .disable_auto_reconnect = false,
        .username = CONFIG_TRIPLES_DEVICE_ID,
        .reconnect_timeout_ms = 3000,
        .keepalive = 60,
        .client_id = device_info.client_id,
        .password = device_info.password,
    // MQTT SSL 连接
#ifdef CONFIG_HUWEI_IOT_ENABLE_MQTTS
        .port = 8883,
        .cert_pem = (const char *)mqtt_org_pem_start,
        .transport = MQTT_TRANSPORT_OVER_SSL,
#else
        .port = 1883,
        .transport = MQTT_TRANSPORT_OVER_TCP,
#endif
    };

    HUAWEI_IOT_INFO("port %d", mqtt_cfg.port);
    HUAWEI_IOT_INFO("host  %s ", mqtt_cfg.host);
    HUAWEI_IOT_INFO("client_id %s ", mqtt_cfg.client_id);
    HUAWEI_IOT_INFO("username %s ", mqtt_cfg.username);
    HUAWEI_IOT_INFO("password%s ", mqtt_cfg.password);

    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);

    vTaskDelete(NULL);
}

esp_err_t mqtt_app_start()
{

    HUAWEI_IOT_INFO("Free memory: %d bytes", esp_get_free_heap_size());

    esp_err_t ret = ESP_FAIL;

    if (xMQTTTaskHandle == NULL)
    {
        BaseType_t xReturn = xTaskCreate(&TaskMQTTConnect, "TaskMQTTConnect", 1024 * 5, NULL, 8, &xMQTTTaskHandle);

        if (xReturn == pdPASS)
        {
            ret = ESP_OK;
            HUAWEI_IOT_INFO("MQTT xTaskCreate OK \n");
        }
        else
        {
            HUAWEI_IOT_INFO("MQTT xTaskCreate Fail : xReturn != pdPASS\n");
        }
    }
    else
    {
        HUAWEI_IOT_INFO("MQTT xTaskCreate Fail: xMQTTTaskHandle !=NULL \n");
    }

    return ret;
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SCAN_DONE)
    {
        HUAWEI_IOT_INFO("Scan done");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_FOUND_CHANNEL)
    {
        HUAWEI_IOT_INFO("Found channel");
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_GOT_SSID_PSWD)
    {
        HUAWEI_IOT_INFO("Got SSID and password");

        smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
        wifi_config_t wifi_config;
        uint8_t ssid[33] = {0};
        uint8_t password[65] = {0};
        uint8_t rvd_data[33] = {0};

        bzero(&wifi_config, sizeof(wifi_config_t));
        memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
        memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
        wifi_config.sta.bssid_set = evt->bssid_set;
        if (wifi_config.sta.bssid_set == true)
        {
            memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
        }

        memcpy(ssid, evt->ssid, sizeof(evt->ssid));
        memcpy(password, evt->password, sizeof(evt->password));
        HUAWEI_IOT_INFO("SSID:%s", ssid);
        HUAWEI_IOT_INFO("PASSWORD:%s", password);

        router_wifi_save_info(ssid, password);

        if (evt->type == SC_TYPE_ESPTOUCH_V2)
        {
            ESP_ERROR_CHECK(esp_smartconfig_get_rvd_data(rvd_data, sizeof(rvd_data)));
            HUAWEI_IOT_INFO("RVD_DATA:");
            for (int i = 0; i < 33; i++)
            {
                printf("%02x ", rvd_data[i]);
            }
            printf("\n");
        }

        ESP_ERROR_CHECK(esp_wifi_disconnect());
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        esp_wifi_connect();
    }
    else if (event_base == SC_EVENT && event_id == SC_EVENT_SEND_ACK_DONE)
    {
        xEventGroupSetBits(wifi_event_group, NET_DONE_BIT);
    }
}

/*
 * @Description:  一键配网
 * @param:
 * @return:
 */
void TaskSmartConfigAirKiss2Net(void *parm)
{
    EventBits_t uxBits;
    //判别是否自动连接
    bool isAutoConnect = routerStartConnect();
    //是的，则不进去配网模式，已连接路由器
    if (isAutoConnect)
    {
        HUAWEI_IOT_INFO("Next connectting router.");
    }
    //否，进去配网模式
    else
    {
        HUAWEI_IOT_INFO("into smartconfig mode");
        ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH_AIRKISS));
        smartconfig_start_config_t cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_smartconfig_start(&cfg));
    }
    //阻塞等待配网完成结果
    while (1)
    {
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | NET_DONE_BIT, true, false, portMAX_DELAY);
        // 配网完成
        if (uxBits & CONNECTED_BIT)
        {
            HUAWEI_IOT_INFO("WiFi Connected to ap");
            mqtt_app_start();
            vTaskDelete(NULL);
        }
        if (uxBits & NET_DONE_BIT)
        {
            HUAWEI_IOT_INFO("smartconfig over");
            esp_smartconfig_stop();
        }
    }
}

/*
 * @Description: 初始化Wifi
 * @param:
 * @return:
 */
static void initialise_wifi(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    xTaskCreate(TaskSmartConfigAirKiss2Net, "TaskSmartConfigAirKiss2Net", 1024 * 3, NULL, 3, NULL);
}

void app_main()
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());
    ESP_LOGI(TAG, "[FW] Release Hard version: %s", CONFIG_HUAWEI_HARDWARE_VERSION);
    ESP_LOGI(TAG, "[FW] Release Soft version: %s", CONFIG_HUAWEI_SOFTWARE_VERSION);

    initialise_wifi();
}

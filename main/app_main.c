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

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
static const int CONNECTED_BIT = BIT0;
static const int ESPTOUCH_DONE_BIT = BIT1;

static TaskHandle_t xMQTTTaskHandle = NULL;
esp_mqtt_client_handle_t client = NULL;

char topicSub[100], topicPub[100], topicPubOTA[100], topicSubOTA[100];
static char *requestId = NULL;

extern const uint8_t mqtt_org_pem_start[] asm("_binary_mqtts_root_cert_pem_start");
extern const uint8_t mqtt_org_pem_end[] asm("_binary_mqtts_root_cert_pem_end");

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

        esp_mqtt_client_subscribe(client, device_info.topic_sub_ota, 1);
        HUAWEI_IOT_INFO("sent subscribe successful=%s", device_info.topic_sub_ota);
        esp_mqtt_client_subscribe(client, device_info.topic_sub_messages, 1);
        HUAWEI_IOT_INFO("sent subscribe successful=%s", device_info.topic_sub_messages);

        // char *bufferOTA = (char *)malloc(BUFFER_LEN);
        // json_generate_version_info(bufferOTA);
        // // 上报版本信息
        // esp_mqtt_client_publish(client, topicPubOTA, bufferOTA, strlen(bufferOTA), 1, 0);
        // free(bufferOTA);
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
        requestId = (char *)malloc(512);
        char *payload = (char *)malloc(1024);
        HUAWEI_IOT_INFO("Free memory: %d bytes", esp_get_free_heap_size());

        snprintf((char *)payload, event->data_len + 1, event->data);

        event->topic[event->topic_len] = '\0';
        event->data[event->data_len] = '\0';

        HUAWEI_IOT_INFO("event->topic[%d]: %s", event->topic_len, event->topic);
        HUAWEI_IOT_INFO("event->data: %s", payload);

        // 首先整体判断是否为一个json格式的数据
        cJSON *pJsonRoot = cJSON_Parse(payload);
        //如果是否json格式数据
        if (pJsonRoot == NULL)
        {
            printf("[SY] Task_ParseJSON_Message xQueueReceive not json ... \n");
            goto __cJSON_Delete;
        }

        //正常数据下发
        if (strcmp(event->topic, topicSub) == 0)
        {
            requestId = (char *)malloc(512);
            cJSON *pJSON_Item_content = cJSON_GetObjectItem(pJsonRoot, "content");
            if (cJSON_IsString(pJSON_Item_content))
            {
                HUAWEI_IOT_INFO("content: %s", pJSON_Item_content->valuestring);
            }
            cJSON *pJSON_Item_id = cJSON_GetObjectItem(pJsonRoot, "id");
            if (cJSON_IsString(pJSON_Item_content))
            {
                memset(requestId, 0, 512);
                sprintf(requestId, "%s", pJSON_Item_id->valuestring);
            }
        }

    __cJSON_Delete:
        cJSON_Delete(pJsonRoot);

        free(requestId);
        free(payload);

        break;
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
        .keepalive = 30,
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
        xEventGroupSetBits(wifi_event_group, ESPTOUCH_DONE_BIT);
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
        uxBits = xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT | ESPTOUCH_DONE_BIT, true, false, portMAX_DELAY);
        // 配网完成
        if (uxBits & CONNECTED_BIT)
        {
            HUAWEI_IOT_INFO("WiFi Connected to ap");
            mqtt_app_start();
            vTaskDelete(NULL);
        }
        if (uxBits & ESPTOUCH_DONE_BIT)
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

    ESP_ERROR_CHECK(nvs_flash_init());
    initialise_wifi();
}

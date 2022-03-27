idf_component_register(SRCS "huawei-iot.c"
                    INCLUDE_DIRS "include"
                    REQUIRES nvs_flash mqtt wpa_supplicant mbedtls)

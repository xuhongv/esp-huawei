idf_component_register(SRCS "huawei-iot.c"
                            "huawei-ota.c"
                            "url_parse.c"
                    INCLUDE_DIRS "include"
                    EMBED_TXTFILES  cert/server_root_cert.pem
                    REQUIRES nvs_flash mqtt app_update wpa_supplicant mbedtls)

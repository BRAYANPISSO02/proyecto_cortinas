idf_component_register(
    SRCS 
        "main.c"
        "components/http_server.c"
        "components/iot_servo.c"
        "components/operation_mode.c"
        "components/sensors_adc.c"
        "components/time_manager.c"
        "components/threshold_manager.c"
        "components/wifi_manager.c"

    INCLUDE_DIRS "." "components"
    EMBED_FILES web_page/styles.css web_page/script.js web_page/index.html
)

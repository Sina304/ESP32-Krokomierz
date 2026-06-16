idf_component_register(
    SRCS "ble_manager.cpp"
    INCLUDE_DIRS "."
    REQUIRES bt nvs_flash esp_driver_gpio
)
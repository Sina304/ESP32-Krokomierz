idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES mpu6050 step_detector ble_manager
)
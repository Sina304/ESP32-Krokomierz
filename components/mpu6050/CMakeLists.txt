idf_component_register(
    SRCS "mpu6050.cpp"
    INCLUDE_DIRS "."
    REQUIRES i2c_bus
)
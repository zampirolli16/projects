# CMakeLists.txt
idf_component_register(SRCS "dht11.c"
PRIV_REQUIRES driver esp_timer
INCLUDE_DIRS ".")
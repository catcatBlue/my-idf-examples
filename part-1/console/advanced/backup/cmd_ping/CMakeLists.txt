idf_component_register(SRCS "cmd_ping.c"
                    INCLUDE_DIRS .
                    REQUIRES "console" "esp_wifi" "iperf"
                             "cmd_wifi" "nvs_flash" "spi_flash" 
                             "vfs" "fatfs" "protocol_examples_common")

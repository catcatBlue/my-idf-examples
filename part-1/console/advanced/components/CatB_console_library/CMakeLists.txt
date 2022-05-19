
idf_component_register(SRCS "CatB_console_library.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "cmd_nvs" "cmd_system" "cmd_wifi" "cmd_ping"
                                "console" "nvs_flash" "spi_flash" 
                                "vfs" "fatfs")
                                # "cmd_ping" "protocol_examples_common"

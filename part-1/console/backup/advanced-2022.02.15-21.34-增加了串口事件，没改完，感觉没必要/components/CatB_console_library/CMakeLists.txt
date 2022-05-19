
idf_component_register(SRCS "CatB_console_library.c" "uart_events.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "cmd_nvs" "cmd_system" "cmd_wifi" 
                                "console" "nvs_flash" "spi_flash" 
                                "vfs" "fatfs" )

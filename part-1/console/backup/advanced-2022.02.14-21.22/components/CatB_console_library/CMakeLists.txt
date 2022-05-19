
idf_component_register(SRCS "CatB_console_library.c" "cmd_wifi.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "cmd_nvs" "cmd_system" "console" "nvs_flash" "spi_flash" "vfs" "fatfs")

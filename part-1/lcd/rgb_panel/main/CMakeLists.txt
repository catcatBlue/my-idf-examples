# idf_component_register(SRCS "rgb_lcd_example_main.c" "lvgl_demo_ui.c"
#                        INCLUDE_DIRS ".")


file(GLOB_RECURSE SOURCES_C *.c)
file(GLOB_RECURSE SOURCES_CPP *.cpp)

idf_component_register(
    SRCS ${SOURCES_C} ${SOURCES_CPP} "rgb_lcd_example_main.c" "lvgl_demo_ui.c"
    INCLUDE_DIRS "."
    REQUIRES driver esp_lcd lvgl lvgl_esp32_drivers
    )
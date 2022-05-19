file(GLOB_RECURSE IMAGE_SOURCES images/*.c)

# idf_component_register(SRCS "i80_controller_example_main.c" "lvgl_demo_ui.c" ${IMAGE_SOURCES}
#                        INCLUDE_DIRS ".")


file(GLOB_RECURSE SOURCES_C *.c)
file(GLOB_RECURSE SOURCES_CPP *.cpp)

idf_component_register(
    SRCS ${SOURCES_C} ${SOURCES_CPP} "i80_controller_example_main.c" "lvgl_demo_ui.c" ${IMAGE_SOURCES}
    INCLUDE_DIRS "."
    REQUIRES driver esp_lcd lvgl lvgl_esp32_drivers
    )
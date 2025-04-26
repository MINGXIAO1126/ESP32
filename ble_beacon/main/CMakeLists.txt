file(GLOB_RECURSE srcs "ble_beacon.c" "src/*.c")

idf_component_register(SRCS "${srcs}"
                       INCLUDE_DIRS "./include"
                       )
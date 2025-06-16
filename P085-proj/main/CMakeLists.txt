file(GLOB_RECURSE srcs "src/*.c")

idf_component_register(SRCS "P085_drv.c" "P085_proj.c" "${srcs}"
                       INCLUDE_DIRS "." "inc")
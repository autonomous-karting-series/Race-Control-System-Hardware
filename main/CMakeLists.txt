# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/main/*.*)
FILE(GLOB_RECURSE nanopb_sources ${CMAKE_SOURCE_DIR}/components/nanopb/*.c)
FILE(GLOB_RECURSE nanopb_gen_sources ${CMAKE_SOURCE_DIR}/components/nanopb/generated/*.pb.c)

idf_component_register(SRCS ${app_sources} ${nanopb_sources} ${nanopb_gen_sources}
    INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include ${CMAKE_SOURCE_DIR}/components/nanopb ${CMAKE_SOURCE_DIR}/components/nanopb/generated
    REQUIRES nvs_flash esp_wifi esp_eth esp_driver_gpio)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error)

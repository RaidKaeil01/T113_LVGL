


aux_source_directory(porting SOURCE)
aux_source_directory(hal SOURCE)
aux_source_directory(porting/device SOURCE)

add_library(lvgl_porting ${SOURCE})

target_include_directories(lvgl_porting PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/porting
    ${CMAKE_CURRENT_LIST_DIR}/porting/device
    ${CMAKE_SOURCE_DIR}/component/hal
)

add_definitions(-DLV_CONF_INCLUDE_SIMPLE)

target_link_libraries(lvgl_porting PRIVATE
    SDL2 m pthread dl freetype z 
)

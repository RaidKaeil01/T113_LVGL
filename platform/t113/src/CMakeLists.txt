
aux_source_directory(porting SOURCE)
aux_source_directory(porting/g2d SOURCE)
aux_source_directory(hal SOURCE)
aux_source_directory(utils SOURCE)

add_library(lvgl_porting ${SOURCE})

target_include_directories(lvgl_porting PUBLIC 
    porting
)

target_include_directories(lvgl_porting PRIVATE 
    hal
    porting
    porting/g2d
    utils
    ${CMAKE_SOURCE_DIR}/lvgl
    ${CMAKE_SOURCE_DIR}/component/hal
)

target_compile_definitions(lvgl_porting PRIVATE 
    -DLV_LVGL_H_INCLUDE_SIMPLE
    -DLV_CONF_INCLUDE_SIMPLE
    -DUSE_SUNXIFB
    -DUSE_SUNXIFB_DOUBLE_BUFFER
    -DUSE_SUNXIFB_CACHE
    -DSUNXIFB_G2D
    -DUSE_SUNXIFB_G2D
    -DUSE_SUNXIFB_G2D_ROTATE
    -DCONF_G2D_VERSION_NEW
    -DLV_USE_SUNXIFB_G2D_FILL
    -DLV_USE_SUNXIFB_G2D_BLEND
    -DLV_USE_SUNXIFB_G2D_BLIT
    -DLV_USE_SUNXIFB_G2D_SCALE
)

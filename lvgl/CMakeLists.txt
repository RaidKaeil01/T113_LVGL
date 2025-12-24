cmake_minimum_required(VERSION 3.15)

# set the project name and version
project(lvgl VERSION 1.0)

aux_source_directory(src SOURCE)
aux_source_directory(src/core SOURCE)
aux_source_directory(src/draw SOURCE)
aux_source_directory(src/draw/nxp_pxp SOURCE)
aux_source_directory(src/draw/nxp_vglite SOURCE)
aux_source_directory(src/draw/sdl SOURCE)
aux_source_directory(src/draw/stm32_dma2d SOURCE)
aux_source_directory(src/draw/sw SOURCE)
aux_source_directory(src/extra SOURCE)
aux_source_directory(src/extra/layouts/flex SOURCE)
aux_source_directory(src/extra/layouts/grid SOURCE)
aux_source_directory(src/extra/libs/ SOURCE)
aux_source_directory(src/extra/libs/bmp SOURCE)
aux_source_directory(src/extra/libs/ffmpeg SOURCE)
aux_source_directory(src/extra/libs/freetype SOURCE)
aux_source_directory(src/extra/libs/fsdrv SOURCE)
aux_source_directory(src/extra/libs/gif SOURCE)
aux_source_directory(src/extra/libs/png SOURCE)
aux_source_directory(src/extra/libs/qrcode SOURCE)
aux_source_directory(src/extra/libs/rlottie SOURCE)
aux_source_directory(src/extra/libs/sjpg SOURCE)
aux_source_directory(src/extra/others/ SOURCE)
aux_source_directory(src/extra/others/gridnav SOURCE)
aux_source_directory(src/extra/others/monkey SOURCE)
aux_source_directory(src/extra/others/snapshot SOURCE)
aux_source_directory(src/extra/others/fragment SOURCE)
aux_source_directory(src/extra/others/imgfont SOURCE)
aux_source_directory(src/extra/others/msg SOURCE)
aux_source_directory(src/extra/others/ime SOURCE)
aux_source_directory(src/extra/themes/ SOURCE)
aux_source_directory(src/extra/themes/basic SOURCE)
aux_source_directory(src/extra/themes/default SOURCE)
aux_source_directory(src/extra/themes/mono SOURCE)
aux_source_directory(src/extra/widgets/ SOURCE)
aux_source_directory(src/extra/widgets/animimg SOURCE)
aux_source_directory(src/extra/widgets/calendar SOURCE)
aux_source_directory(src/extra/widgets/chart SOURCE)
aux_source_directory(src/extra/widgets/colorwheel SOURCE)
aux_source_directory(src/extra/widgets/imgbtn SOURCE)
aux_source_directory(src/extra/widgets/keyboard SOURCE)
aux_source_directory(src/extra/widgets/led SOURCE)
aux_source_directory(src/extra/widgets/list SOURCE)
aux_source_directory(src/extra/widgets/menu SOURCE)
aux_source_directory(src/extra/widgets/meter SOURCE)
aux_source_directory(src/extra/widgets/msgbox SOURCE)
aux_source_directory(src/extra/widgets/span SOURCE)
aux_source_directory(src/extra/widgets/spinbox SOURCE)
aux_source_directory(src/extra/widgets/spinner SOURCE)
aux_source_directory(src/extra/widgets/tabview SOURCE)
aux_source_directory(src/extra/widgets/tileview SOURCE)
aux_source_directory(src/extra/widgets/win SOURCE)
aux_source_directory(src/font SOURCE)
aux_source_directory(src/gpu SOURCE)
aux_source_directory(src/hal SOURCE)
aux_source_directory(src/misc SOURCE)
aux_source_directory(src/widgets SOURCE)

add_library(lvgl ${SOURCE})

set(INC
    src 
    src/core 
    src/draw 
    src/draw/nxp_pxp 
    src/draw/nxp_vglite 
    src/draw/sdl 
    src/draw/stm32_dma2d 
    src/draw/sw 
    src/extra 
    src/extra/layouts/flex 
    src/extra/layouts/grid 
    src/extra/libs/ 
    src/extra/libs/bmp 
    src/extra/libs/ffmpeg 
    src/extra/libs/freetype 
    src/extra/libs/fsdrv 
    src/extra/libs/gif 
    src/extra/libs/png 
    src/extra/libs/qrcode 
    src/extra/libs/rlottie 
    src/extra/libs/sjpg 
    src/extra/others/ 
    src/extra/others/gridnav 
    src/extra/others/monkey 
    src/extra/others/snapshot 
    src/extra/others/fragment
    src/extra/others/imgfont
    src/extra/others/msg
    src/extra/others/ime
    src/extra/themes/ 
    src/extra/themes/basic 
    src/extra/themes/default 
    src/extra/themes/mono 
    src/extra/widgets/ 
    src/extra/widgets/animimg 
    src/extra/widgets/calendar 
    src/extra/widgets/chart 
    src/extra/widgets/colorwheel 
    src/extra/widgets/imgbtn 
    src/extra/widgets/keyboard 
    src/extra/widgets/led 
    src/extra/widgets/list 
    src/extra/widgets/menu 
    src/extra/widgets/meter 
    src/extra/widgets/msgbox 
    src/extra/widgets/span 
    src/extra/widgets/spinbox 
    src/extra/widgets/spinner 
    src/extra/widgets/tabview 
    src/extra/widgets/tileview 
    src/extra/widgets/win 
    src/font 
    src/gpu 
    src/hal 
    src/misc 
    src/widgets
)

include_directories(
    ${INC}
)

target_include_directories(lvgl PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/..
    ${CMAKE_CURRENT_LIST_DIR}
    ${INC}
)

target_link_libraries(lvgl PUBLIC
    lvgl_porting
)

target_compile_definitions(lvgl PUBLIC
    -DLV_CONF_INCLUDE_SIMPLE
)

aux_source_directory(demos/benchmark DEMOS)
aux_source_directory(demos/benchmark/assets DEMOS)
aux_source_directory(demos/keypad_encoder DEMOS)
aux_source_directory(demos/music DEMOS)
aux_source_directory(demos/music/assets DEMOS)
aux_source_directory(demos/stress DEMOS)
aux_source_directory(demos/stress/assets DEMOS)
aux_source_directory(demos/widgets DEMOS)
aux_source_directory(demos/widgets/assets DEMOS)
aux_source_directory(demos/ DEMOS)

add_library(lvgl_demos ${DEMOS})

target_compile_definitions(lvgl_demos PRIVATE
    -DLV_CONF_INCLUDE_SIMPLE
    -DLV_LVGL_H_INCLUDE_SIMPLE
    -DLV_DEMO_CONF_INCLUDE_SIMPLE
)

target_include_directories(lvgl_demos PUBLIC 
    ${CMAKE_CURRENT_LIST_DIR}/demos
)

target_include_directories(lvgl_demos PRIVATE 
    ${CMAKE_CURRENT_LIST_DIR}
    demos/lv_demo_benchmark
    demos/lv_demo_benchmark/assets
    demos/lv_demo_keypad_encoder
    demos/lv_demo_music
    demos/lv_demo_music/assets
    demos/lv_demo_stress
    demos/lv_demo_stress/assets
    demos/lv_demo_widgets
    demos/lv_demo_widgets/assets
)

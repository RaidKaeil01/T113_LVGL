
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)

# 新版本仅需修改build.sh的编译器路径即可，这里无需添加

set(CMAKE_C_COMPILER ${TOOLCHAIN_PATH}/arm-openwrt-linux-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/arm-openwrt-linux-g++)

add_link_options(-L${CMAKE_CURRENT_LIST_DIR}/lib)
add_link_options(-L${CMAKE_SOURCE_DIR}/wifi/libs/)
add_link_options(-L${CMAKE_SOURCE_DIR}/net/libs/)

add_link_options(-lpthread -lfreetype -lrt -ldl -znow -zrelro -luapi -lm -lz -lbz2 -O0 -rdynamic -g -funwind-tables -ffunction-sections)
add_link_options(-fPIC -Wl,-gc-sections)
add_compile_options(-I${CMAKE_CURRENT_LIST_DIR}/src/porting)
add_compile_options(-I${CMAKE_SOURCE_DIR})
add_compile_options(-I${CMAKE_SOURCE_DIR}/lvgl/demos)
add_compile_options(-I${CMAKE_CURRENT_LIST_DIR}/include)
add_compile_options(-I${CMAKE_CURRENT_LIST_DIR}/include/freetype)

add_compile_options(-march=armv7-a -mtune=cortex-a7 -mfpu=neon -mfloat-abi=hard -O0 -ldl -rdynamic -g -funwind-tables -ffunction-sections)
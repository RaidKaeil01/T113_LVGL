cmake_minimum_required(VERSION 3.15)
# 工程名称demo1
project(demo7)

 #添加显式定义，定义宏 SIMULATOR_LINUX,这样才能在代码中识别到该宏
if(SIMULATOR_LINUX)
    add_compile_definitions(SIMULATOR_LINUX)  
endif()
# 拷贝res文件夹到build中，方便后续打包
file(COPY res DESTINATION ${PROJECT_BINARY_DIR})

# 编译当前目录下所有.c文件
aux_source_directory(. SOURCES)
# 添加 wifi 子目录的源文件
aux_source_directory(wifi WIFI_SOURCES)
# 添加 net 子目录的源文件
aux_source_directory(net NET_SOURCES)
# 生成名称为demo7的可执行文件
add_executable(demo7 ${SOURCES} ${WIFI_SOURCES} ${NET_SOURCES})
# 引入头文件路径
target_include_directories(demo7 PRIVATE
                                ./
                                ./wifi
                                ./net
                                res/
                                res/font/
                                res/image/
                                ${CMAKE_SOURCE_DIR}/lvgl/demos
                                ${CMAKE_SOURCE_DIR}/component/font
                                ${CMAKE_SOURCE_DIR}/component/osal
                                ${CMAKE_SOURCE_DIR}/lvgl
                                ${CMAKE_SOURCE_DIR}/component/wifi
                                )
# 引入依赖库 
target_link_libraries(demo7 PRIVATE
                        lvgl_demos
                        font
                        lvgl
                        wifi
                        osal
                        cjson
                        curl 
                        ssl
                        crypto
                        nghttp2
                        pthread
                        )
# 当使用 PRIVATE 关键字时，表示所链接的库仅用于当前目标的实现，而不会传递给依赖该目标的其他目标。
# 换句话说：
# 当前目标（demo1）会链接这些库并使用它们。
# 其他依赖 demo1的目标（比如另一个库或可执行文件）不会自动链接这些库。
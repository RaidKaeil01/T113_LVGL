# 指定最低 CMake 版本为 3.15
cmake_minimum_required(VERSION 3.15)

# 定义项目名称为 demo
project(demo)

# 添加资源子目录（包含字体、图片、音乐等资源文件）
add_subdirectory(res)

# 自动收集源文件到 SOURCES 变量
aux_source_directory(. SOURCES)                    # 当前目录（main.c）
aux_source_directory(ui SOURCES)                   # UI 页面文件（page_*.c）
aux_source_directory(ui/data SOURCES)              # 数据管理（device_data.c）
aux_source_directory(ui/utils SOURCES)             # 工具函数（utils.c, ble_mesh.c）
aux_source_directory(ui/utils/clock SOURCES)       # 时钟相关工具
aux_source_directory(ui/utils/file SOURCES)        # 文件操作工具

# 使用收集到的源文件创建可执行程序 demo
# 使用收集到的源文件创建可执行程序 demo
add_executable(demo ${SOURCES})

# 设置头文件搜索路径（告诉编译器在哪些目录查找 .h 文件）
target_include_directories(demo PRIVATE
                        ./                                     # 当前目录
                        ui/                                    # UI 头文件
                        ui/data/                               # 数据层头文件
                        ui/utils/                              # 工具头文件
                        ui/utils/clock/                        # 时钟工具头文件
                        ui/utils/file/                         # 文件工具头文件
                        res/                                   # 资源配置
                        res/color/                             # 颜色配置
                        res/font/                              # 字体配置
                        res/music/                             # 音乐配置
                        res/image/                             # 图片配置
                        ${CMAKE_SOURCE_DIR}/lvgl               # LVGL 库根目录
                        ${CMAKE_SOURCE_DIR}/lvgl/src           # LVGL 源码
                        ${CMAKE_SOURCE_DIR}/lvgl/demos         # LVGL 演示
                        ${CMAKE_SOURCE_DIR}/component/hal      # 硬件抽象层
                        ${CMAKE_SOURCE_DIR}/component/wifi     # WiFi 管理
                        ${CMAKE_SOURCE_DIR}/component/net      # 网络管理
                        ${CMAKE_SOURCE_DIR}/component/osal     # 操作系统抽象层
                        ${CMAKE_SOURCE_DIR}/component/font     # 字体工具
                        ${CMAKE_SOURCE_DIR}/component/player   # 音频播放
                        ${CMAKE_SOURCE_DIR}/component/usb_hid  # USB HID
                        ${CMAKE_SOURCE_DIR}/component/net/inc  # 网络头文件
)

# 链接库文件（将编译好的库链接到 demo 程序）
target_link_libraries(demo PRIVATE
                        lvgl_demos      # LVGL 演示库
                        lvgl            # LVGL 核心库
                        lvgl_porting    # LVGL 平台移植层
                        wifi            # WiFi 功能库
                        http            # HTTP 请求库
                        player          # 音频播放库
                        font            # 字体处理库
                        usb_hid         # USB HID 库
                        )


# 根据编译目标设置不同的资源路径和编译宏
if(SIMULATOR_LINUX)
    # Linux 模拟器模式：定义 SIMULATOR_LINUX 宏，资源路径在构建目录下
    add_compile_definitions(SIMULATOR_LINUX)
    set(PROJECT_RES_URL ${PROJECT_BINARY_DIR}/res/)
else()
    # T113 硬件模式：资源路径在设备的 /usr/res/ 目录
    set(PROJECT_RES_URL /usr/res/)
endif()

# 将资源路径定义为编译宏，代码中可以使用 PROJECT_RES_URL 访问
target_compile_definitions(demo PUBLIC PROJECT_RES_URL="${PROJECT_RES_URL}")

# 打印资源路径，方便编译时查看
message("PROJECT_RES_URL is ${PROJECT_RES_URL}")



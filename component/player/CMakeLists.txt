project(player)

aux_source_directory(. SOURCE)

include_directories(player
    PRIVATE
    .
    ${CMAKE_SOURCE_DIR}/component/hal
    ${CMAKE_SOURCE_DIR}/component/osal
)

add_library(player STATIC ${SOURCE})

target_link_libraries(player 
                lvgl_porting
                osal
                )     
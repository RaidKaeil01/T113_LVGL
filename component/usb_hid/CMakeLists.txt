project(usb_hid)

aux_source_directory(. SOURCE)

include_directories(usb_hid
    PRIVATE
    .
    ${CMAKE_SOURCE_DIR}/component/hal
    ${CMAKE_SOURCE_DIR}/component/osal
)

add_library(usb_hid STATIC ${SOURCE})

target_link_libraries(usb_hid 
                lvgl_porting
                osal
                )     
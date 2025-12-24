project(wifi)

aux_source_directory(./ SOURCE)
aux_source_directory(./osal SOURCE)
aux_source_directory(./utils SOURCE)

include_directories(wifi PUBLIC
                            .
                            ./osal
                            ./utils
                            ${CMAKE_SOURCE_DIR}/component/osal
                            )


add_library(wifi STATIC ${SOURCE})

link_directories(./libs)

target_link_libraries(wifi wpa_client osal)

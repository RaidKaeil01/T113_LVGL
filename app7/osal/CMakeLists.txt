project(osal)

aux_source_directory(./ SOURCE)

include_directories(osal PUBLIC
                        .
                        )

add_library(osal STATIC ${SOURCE})


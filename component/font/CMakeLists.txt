project(font)
aux_source_directory(./ SOURCE)
include_directories(font PUBLIC
                        .
                        )
add_library(font STATIC ${SOURCE})
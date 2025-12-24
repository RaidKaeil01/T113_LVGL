
set(CMAKE_SYSTEM_NAME Linux)

set(CMAKE_C_COMPILER /usr/bin/gcc)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
set(CMAKE_ASM_COMPILER /usr/bin/gcc)

set(CMAKE_C_FLAGS "-g")

add_link_options(-L${CMAKE_CURRENT_LIST_DIR}/lib)
add_link_options(-pthread)

add_compile_options(-I${CMAKE_CURRENT_LIST_DIR}/src/porting)
add_compile_options(-I${CMAKE_SOURCE_DIR})
add_compile_options(-I${CMAKE_SOURCE_DIR}/lvgl/demos)
add_compile_options(-I${CMAKE_CURRENT_LIST_DIR}/include)
